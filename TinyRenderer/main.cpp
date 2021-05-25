#define _CRT_SECURE_NO_WARNING
#define _USE_MATH_DEFINES

#include <math.h>
#include <cstdlib>
#include <iostream>
#include "geometry.h"
#include "bmpimage.h"
#include "tgaimage.h"
#include "model.h"
#include "rasterize.h"
#include "shader.h"
#include "zbuffer.h"
#include <string>
#include "matrix.h"

const int height = 800;
const int width = 800;

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

Vec4f perspectiveDivide(Vec4f v) {
    for (int i = 0; i < 4; i++) {
        v[i] = v[i] / v[3];
    }
    return v;
}

struct Shader:public IShader {

private:
    Model* model;
    TGAImage* texture;
    TGAImage* normalMapTangentTexture;
    BMPImage* shadowMap;
    Matrix screenToShadowScreenMatrix;

public:
    Shader(Model* m, TGAImage* t, TGAImage* nm, BMPImage* sm, Matrix stssm) {
        model = m;
        texture = t;
        normalMapTangentTexture = nm;
        shadowMap = sm;
        screenToShadowScreenMatrix = stssm;
    }

    Vec3f lightDirection = Vec3f(1, 1, 1).normalize();
    Vec3f varyingIntensity;
    mat<3, 3, float> varyingNormal; 
    mat<2, 3, float> varyingUv;

    // output should be clip-space coordinates, before perspective-divide (hardware will do the divide/viewport transform)
    virtual Vec4f vertex(int faceIndex, int vertIndex) {
        Vec3f modelCoordinates = model->vert(model->face(faceIndex)[vertIndex]); // read the vertex from .obj file

        // normal
        Vec3f normal = model->normal(model->face(faceIndex)[vertIndex]); // TODO: transform normals by inverse transpose of Projection * ModelView
        Vec4f normalHomogenous = vec3fToVec4fVector(normal);
        normalHomogenous = (Projection * ModelView).invert_transpose() * normalHomogenous;
        normal = vec4fToVec3f(normalHomogenous);
        varyingNormal.set_col(vertIndex, normal);

        // uv
        Vec2f uvCoordinates = model->uv(model->face_uv_indices(faceIndex)[vertIndex]); // read the UV coordinates from the .obj file
        varyingUv.set_col(vertIndex, uvCoordinates);

        Vec4f gl_Vertex = vec3fToVec4fPoint(modelCoordinates);

        varyingIntensity[vertIndex] = std::max(0.f, model->normal(model->face(faceIndex)[vertIndex]) * lightDirection); // get diffuse lighting intensity

        return Projection * ModelView * gl_Vertex;
    }
    
    virtual bool fragment(mat<3, 3, float> normalizedDeviceCoordinates, mat<3, 3, float> screenCoordinates, Vec3f barycentricCoodinates, BMPColor& color) {
        // uv
        Vec2f uv = varyingUv * barycentricCoodinates;
        int textureX = uv.x * texture->get_width();
        int textureY = uv.y * texture->get_height();
        TGAColor textureColor = texture->get(textureX, textureY);

        // normal
        Vec3f fragmentNormal = (varyingNormal * barycentricCoodinates).normalize();

        // compute frame tangent to surface (Darboux frame)
        mat<3, 3, float> aMatrix;

        Vec3f p0p1 = normalizedDeviceCoordinates.col(1) - normalizedDeviceCoordinates.col(0);
        Vec3f p0p2 = normalizedDeviceCoordinates.col(2) - normalizedDeviceCoordinates.col(0);

        aMatrix[0] = p0p1;
        aMatrix[1] = p0p2;
        aMatrix[2] = fragmentNormal;

        mat<3, 3, float> aInverseMatrix = aMatrix.invert();

        // basis vectors
        Vec3f tangent = (aInverseMatrix * Vec3f(varyingUv[0][1] - varyingUv[0][0], varyingUv[0][2] - varyingUv[0][0], 0)).normalize(); // u1 - u0
        Vec3f bitangent = (aInverseMatrix * Vec3f(varyingUv[1][1] - varyingUv[1][0], varyingUv[1][2] - varyingUv[1][0], 0)).normalize(); // v1 - v0

        // tangent normal
        TGAColor tangentNormalColor = normalMapTangentTexture->get(textureX, textureY);
        Vec3f tangentNormal = Vec3f((float) tangentNormalColor.r / 255.f * 2.f - 1.f, (float)tangentNormalColor.g / 255.f * 2.f - 1.f, (float)tangentNormalColor.b / 255.f * 2.f - 1.f);

        mat<3, 3, float> normalTextureToWorldCoordinates;
        normalTextureToWorldCoordinates.set_col(0, tangent);
        normalTextureToWorldCoordinates.set_col(1, bitangent);
        normalTextureToWorldCoordinates.set_col(2, fragmentNormal);

        // transform from normal texture space to world space
        Vec3f textureNormal = (normalTextureToWorldCoordinates * tangentNormal).normalize();

        float intensity = std::min(1.0f, std::max(0.0f, textureNormal * lightDirection + 0.2f));

        // check if the shadow camera can see this point, if not, then the point is in a shadow
        Vec4f screenCoordinate = vec3fToVec4fPoint(screenCoordinates * barycentricCoodinates);
        Vec4f screenCoordinateShadow = screenToShadowScreenMatrix * screenCoordinate;
        screenCoordinateShadow = perspectiveDivide(screenCoordinateShadow);
        int screenCoordinateShadowValue = screenCoordinateShadow[2] / zBufferDepth * 255;
        BMPColor shadowMapValue = shadowMap->get(screenCoordinateShadow[0], screenCoordinateShadow[1]);

        if (shadowMapValue.r > screenCoordinateShadowValue) {
            // shadow map depth is higher than our depth, meaning there's something blocking this point
            intensity *= 0.1;
        }

        color = BMPColor(textureColor.r * intensity, textureColor.g * intensity, textureColor.b * intensity, 255);
        return false; // do not discard pixel
    }
};

struct ShadowMapShader :public IShader {

private:
    Model* model;

public:
    ShadowMapShader(Model* m) {
        model = m;
    }

    // output should be clip-space coordinates, before perspective-divide (hardware will do the divide/viewport transform)
    virtual Vec4f vertex(int faceIndex, int vertIndex) {
        Vec3f modelCoordinates = model->vert(model->face(faceIndex)[vertIndex]); // read the vertex from .obj file
        Vec4f gl_Vertex = vec3fToVec4fPoint(modelCoordinates);
        return Projection * ModelView * gl_Vertex;
    }

    virtual bool fragment(mat<3, 3, float> normalizedDeviceCoordinates, mat<3, 3, float> screenCoordinates, Vec3f barycentricCoodinates, BMPColor& color) {
        Vec3f fragmentScreenCoordinates = screenCoordinates * barycentricCoodinates;
        
        float intensity = fragmentScreenCoordinates.z / zBufferDepth;
        color = BMPColor(255 * intensity, 255 * intensity, 255 * intensity, 255);
        return false; // do not discard pixel
    }
};

void draw(Model* model, IShader& shader, Vec3f cameraDirection, BMPImage frameBuffer, int* zBuffer) {
    // the render loop
    for (int i = 0; i < model->nfaces(); i++) {
        mat<3, 3, float> screenCoordinates;
        mat<3, 3, float> normalizedDeviceCoordinates;

        // run vertex shader
        for (int j = 0; j < 3; j++) {
            Vec4f clipCoordinates = shader.vertex(i, j);

            // perspective divide
            Vec4f ndc = perspectiveDivide(clipCoordinates);
            normalizedDeviceCoordinates.set_col(j, vec4fToVec3f(ndc));

            // viewport transform
            screenCoordinates.set_col(j, vec4fToVec3f(Viewport * ndc));
        }

        // backface culling
        Vec3f faceNormal = cross(screenCoordinates.col(1) - screenCoordinates.col(0), screenCoordinates.col(2) - screenCoordinates.col(0));
        faceNormal.normalize();

        if ((faceNormal * cameraDirection) > 0) { // negative value here means camera and surface are pointing towards each other (--> <--)
            continue;
        }

        // rasterize (calls fragment shader)
        rasterize(normalizedDeviceCoordinates, screenCoordinates, frameBuffer, zBuffer, shader);
    }
}

/*
Current Goal: 
- add blinn-phong specular stuff
*/
int main(int argc, char** argv) {

    // rendering data structures
    BMPImage frameBuffer(height, width, 3);
    BMPImage shadowMapFrameBuffer(height, width, 3);
    int* zBuffer = initZBuffer(frameBuffer.getWidth(), frameBuffer.getHeight());
    int* shadowMapZBuffer = initZBuffer(shadowMapFrameBuffer.getWidth(), shadowMapFrameBuffer.getHeight());

    // camera
    float cameraPlaneDistance = 10.0;
    Projection = projectionMatrix(cameraPlaneDistance);
    Vec3f cameraDirection = Vec3f(0, 0, -1); // camera is always looking down -Z axis
    Vec3f up = Vec3f(0, 1, 0);
    Vec3f lookAtPoint = Vec3f(0, 0, 0);
    Viewport = viewportMatrix(width / 8.0, height / 8.0, width * 3.0 / 4.0, height * 3.0 / 4.0);

    // model/texture
    Model* model = new Model("obj/diablo3_pose.obj");
    TGAImage modelTexture = TGAImage();
    modelTexture.read_tga_file("texture/diablo3_pose_diffuse.tga");
    modelTexture.flip_vertically();

    TGAImage normalMapTangentTexture = TGAImage();
    normalMapTangentTexture.read_tga_file("texture/diablo3_pose_nm_tangent.tga");
    normalMapTangentTexture.flip_vertically();
    
    // render shadow map
    Vec3f cameraPos = Vec3f(1, 1, 1);
    ModelView = viewMatrix(cameraPos, lookAtPoint, up);

    ShadowMapShader shadowMapShader = ShadowMapShader(model);
    draw(model, shadowMapShader, cameraDirection, shadowMapFrameBuffer, shadowMapZBuffer);
    Matrix shadowModelToScreenMatrix = Viewport * Projection * ModelView;

    // render main scene
    cameraPos = Vec3f(0, 0, 1);
    ModelView = viewMatrix(cameraPos, lookAtPoint, up);

    // goes from main scene screen coordinates to shadow screen coordinates
    // works by first moving back to object coordinates then moving to shadow screen coordinates
    Matrix screenToShadowScreenMatrix = shadowModelToScreenMatrix * (Viewport * Projection * ModelView).invert();

    Shader shader = Shader(model, &modelTexture, &normalMapTangentTexture, &shadowMapFrameBuffer, screenToShadowScreenMatrix);
    draw(model, shader, cameraDirection, frameBuffer, zBuffer);

    // output images
    std::string imageFileName = "bitmapImage.bmp";
    frameBuffer.save(&imageFileName[0]);

    BMPImage zBufferImage = zBufferToImage(zBuffer, width, height);
    imageFileName = "bitmapImageZBuffer.bmp";
    zBufferImage.save(&imageFileName[0]);

    imageFileName = "bitmapImageShadowMap.bmp";
    shadowMapFrameBuffer.save(&imageFileName[0]);

    std::cout << "Image generated!!";
    delete model;
    return 0;
}