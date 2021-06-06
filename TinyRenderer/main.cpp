#define _CRT_SECURE_NO_WARNING
#define _USE_MATH_DEFINES

#include <math.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include "geometry.h"
#include "bmpimage.h"
#include "tgaimage.h"
#include "model.h"
#include "rasterize.h"
#include "shader.h"
#include "zbuffer.h"
#include "matrix.h"
#include "occlusion.h"
#include "util.h"

const int height = 800;
const int width = 800;
Vec3f mainCameraPos = Vec3f(0, 0, 3);
BMPColor RED = BMPColor(255, 0, 0, 255);

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

struct Shader:public IShader {

private:
    Model* model;
    TGAImage* texture;
    TGAImage* specularTexture;
    TGAImage* normalMapTangentTexture;
    float* zBuffer;
    float* shadowMap;
    Matrix screenToShadowScreenMatrix;

public:
    Shader(Model* m, TGAImage* t, TGAImage* st, TGAImage* nm, float* zb, float* sm, Matrix stssm) {
        model = m;
        texture = t;
        specularTexture = st;
        normalMapTangentTexture = nm;
        zBuffer = zb;
        shadowMap = sm;
        screenToShadowScreenMatrix = stssm;
    }

    Vec3f lightDirection = Vec3f(1, 1, 1).normalize();
    Vec3f varyingIntensity;
    mat<3, 3, float> varyingModelCoordinates;
    mat<3, 3, float> varyingNormal; 
    mat<2, 3, float> varyingUv;

    // output should be clip-space coordinates, before perspective-divide (hardware will do the divide/viewport transform)
    virtual Vec4f vertex(int faceIndex, int vertIndex) {
        Vec3f modelCoordinates = model->vert(model->face(faceIndex)[vertIndex]); // read the vertex from .obj file
        varyingModelCoordinates.set_col(vertIndex, modelCoordinates);

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
    
    virtual bool fragment(int fragX, int fragY, mat<3, 3, float> normalizedDeviceCoordinates, mat<3, 3, float> screenCoordinates, Vec3f barycentricCoodinates, BMPColor& color) {
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

        // lighting calculations (Phong) ambient + diffuse + specular
        float ambient = 5;
        
        Vec3f modelCoordinate = varyingModelCoordinates * barycentricCoodinates; // world coords of this fragment
        Vec3f l = vec4fToVec3f(Projection * ModelView * vec3fToVec4fVector(lightDirection)).normalize();

        float diffuse = std::max(textureNormal * l, 0.0f);

        Vec3f v = (mainCameraPos - modelCoordinate).normalize(); // camera - current point
        Vec3f r = (textureNormal * (2.0f * (l * textureNormal)) - l).normalize(); // light vector reflected about normal
        float specularMapValue = specularTexture->get(textureX, textureY).r / 255.0f;
        float specular = pow(std::max(r * v, 0.0f), 1) * 10;
        float intensity = diffuse + specularMapValue * specular;

        // shadows
        
        // check if the shadow camera can see this point, if not, then the point is in a shadow
        Vec4f screenCoordinate = vec3fToVec4fPoint(screenCoordinates * barycentricCoodinates);
        Vec4f screenCoordinateShadow = screenToShadowScreenMatrix * screenCoordinate;
        screenCoordinateShadow = perspectiveDivide(screenCoordinateShadow);

        float screenCoordinateShadowValue = screenCoordinateShadow[2];
        int shadowMapIndex = int(screenCoordinateShadow[0]) + int(screenCoordinateShadow[1]) * width;
        float shadowMapValue = shadowMap[shadowMapIndex];

        float bias = 0.005 * tan(acos(std::min(diffuse, 1.0f))); // cosTheta is dot( n,l ), clamped between 0 and 1
        float shadow = 1.0;
        if (shadowMapValue - screenCoordinateShadowValue > 0.1) { // negative farther back in depth
          // shadow map depth is higher than our depth, meaning there's something blocking this point
          intensity *= 0.05;
          shadow = 0.05;
        }

        // ambient occlusion (screen-space)
        float totalVisibility = 0;
        for (float angle = 0; angle < (M_PI * 2 - M_PI / 8); angle += M_PI / 4) { // cast 8 rays out from this point
            // in theory we want solid angle so we do 90 - maxElevationAngle to get the inner FOV seen from this point
            int dirX = signum(cos(angle));
            int dirY = signum(sin(angle));
            float vis = M_PI / 2 - maxElevationAngle(fragX, fragY, dirX, dirY, zBuffer, width, height);
            totalVisibility += vis;
        }

        float percentVisibility = totalVisibility / ((M_PI / 2) * 8); // 90 * 8 would be the max field of view
        percentVisibility = std::pow(percentVisibility, 300.0f); // exponent to increase contrast of final image

        color = BMPColor(
            std::min(ambient + intensity * textureColor.r, 255.f),
            std::min(ambient + intensity * textureColor.g, 255.f),
            std::min(ambient + intensity * textureColor.b, 255.f),
            255);

        return false; // do not discard pixel
    }
};

struct DepthMapShader :public IShader {

private:
    Model* model;

public:
    DepthMapShader(Model* m) {
        model = m;
    }

    // output should be clip-space coordinates, before perspective-divide (hardware will do the divide/viewport transform)
    virtual Vec4f vertex(int faceIndex, int vertIndex) {
        Vec3f modelCoordinates = model->vert(model->face(faceIndex)[vertIndex]); // read the vertex from .obj file
        Vec4f gl_Vertex = vec3fToVec4fPoint(modelCoordinates);
        return Projection * ModelView * gl_Vertex;
    }

    virtual bool fragment(int fragX, int fragY, mat<3, 3, float> normalizedDeviceCoordinates, mat<3, 3, float> screenCoordinates, Vec3f barycentricCoodinates, BMPColor& color) {
        return true; // don't care about the image buffer, just the zBuffer
    }
};

void draw(Model* model, IShader& shader, Vec3f cameraDirection, BMPImage frameBuffer, float* zBuffer) {
    // the render loop
    for (int i = 0; i < model->nfaces(); i++) {
        mat<3, 3, float> clipCoordinates;
        mat<3, 3, float> screenCoordinates;
        mat<3, 3, float> normalizedDeviceCoordinates;

        std::cout << "Draw call: " << (float) i / (float) model->nfaces() * 100 << "%\n";

        // run vertex shader
        for (int j = 0; j < 3; j++) {
            Vec4f clipCoordinate = shader.vertex(i, j);
            clipCoordinates.set_col(j, vec4fToVec3f(clipCoordinate));

            // perspective divide
            Vec4f ndc = perspectiveDivide(clipCoordinate);
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

int main(int argc, char** argv) {

    // rendering data structures
    BMPImage frameBuffer(height, width, 3);
    BMPImage depthFrameBuffer(height, width, 3);
    BMPImage shadowMapFrameBuffer(height, width, 3);
    float* zBufferFirstPass = initZBuffer(frameBuffer.getWidth(), frameBuffer.getHeight());
    float* zBuffer = initZBuffer(frameBuffer.getWidth(), frameBuffer.getHeight());
    float* shadowMapZBuffer = initZBuffer(shadowMapFrameBuffer.getWidth(), shadowMapFrameBuffer.getHeight());
     
    // camera
    Vec3f cameraDirection = Vec3f(0, 0, -1); // camera is always looking down -Z axis
    Vec3f up = Vec3f(0, 1, 0);
    Vec3f lookAtPoint = Vec3f(0, 0, 0);
    float cameraPlaneDistance = (lookAtPoint - mainCameraPos).norm();
    Projection = projectionMatrix(cameraPlaneDistance);
    Viewport = viewportMatrix(width / 8.0, height / 8.0, width * 3.0 / 4.0, height * 3.0 / 4.0);

    // model/texture
    Model* model = new Model("obj/diablo3_pose.obj");
    TGAImage modelTexture = TGAImage();
    modelTexture.read_tga_file("texture/diablo3_pose_diffuse.tga");
    modelTexture.flip_vertically();

    TGAImage specularTexture = TGAImage();
    specularTexture.read_tga_file("texture/diablo3_pose_spec.tga");
    specularTexture.flip_vertically();

    TGAImage normalMapTangentTexture = TGAImage();
    normalMapTangentTexture.read_tga_file("texture/diablo3_pose_nm_tangent.tga");
    normalMapTangentTexture.flip_vertically();
    
    // render shadow map
    Vec3f shadowCameraPos = Vec3f(0, 3, 3);
    ModelView = viewMatrix(shadowCameraPos, lookAtPoint, up);

    DepthMapShader shadowMapShader = DepthMapShader(model);
    draw(model, shadowMapShader, cameraDirection, shadowMapFrameBuffer, shadowMapZBuffer);
    Matrix shadowModelToScreenMatrix = Viewport * Projection * ModelView;

    // render depth pass
    ModelView = viewMatrix(mainCameraPos, lookAtPoint, up);
    DepthMapShader depthMapShader = DepthMapShader(model);
    draw(model, depthMapShader, cameraDirection, depthFrameBuffer, zBufferFirstPass);

    // render main scene
    
    // goes from main scene screen coordinates to shadow screen coordinates
    // works by first moving back to object coordinates then moving to shadow screen coordinates
    Matrix screenToShadowScreenMatrix = shadowModelToScreenMatrix * (Viewport * Projection * ModelView).invert();

    Shader shader = Shader(model, &modelTexture, &specularTexture, &normalMapTangentTexture, zBufferFirstPass, shadowMapZBuffer, screenToShadowScreenMatrix);
    draw(model, shader, cameraDirection, frameBuffer, zBuffer);

    // output images
    BMPImage zBufferImage = zBufferToImage(zBufferFirstPass, width, height);
    zBufferImage.set(332, 316, RED);
    BMPImage shadowMapZBufferImage = zBufferToImage(shadowMapZBuffer, width, height);
    shadowMapZBufferImage.set(333, 324, RED);

    std::string imageFileName = "bitmapImageShadowMap.bmp";
    shadowMapZBufferImage.save(&imageFileName[0]);

    imageFileName = "bitmapImageZBuffer.bmp";
    zBufferImage.save(&imageFileName[0]);

    imageFileName = "bitmapImage.bmp";
    frameBuffer.save(&imageFileName[0]);

    std::cout << "Image generated!!";
    delete model;
    return 0;
}