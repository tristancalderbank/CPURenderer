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

const BMPColor white = BMPColor(255, 255, 255, 255);
const BMPColor grey = BMPColor(128, 128, 128, 255);
const BMPColor red = BMPColor(255, 0, 0, 255);
const BMPColor blue = BMPColor(0, 0, 255, 255);
const BMPColor green = BMPColor(0, 255, 0, 255);

const int height = 800;
const int width = 800;

Matrix ModelView;
Matrix Viewport;
Matrix Projection;

struct Shader:public IShader {

private:
    Model* model;
    TGAImage* texture;

public:
    Shader(Model* m, TGAImage* t) {
        model = m;
        texture = t;
    }

    Vec3f lightDirection = Vec3f(0, 0, 1);
    Vec3f varyingIntensity; // written by vertex shader, read by fragment shader
    mat<2, 3, float> varyingUv;

    // output should be clip-space coordinates, before perspective-divide (hardware will do the divide/viewport transform)
    virtual Vec4f vertex(int faceIndex, int vertIndex) {
        Vec3f modelCoordinates = model->vert(model->face(faceIndex)[vertIndex]); // read the vertex from .obj file

        Vec2f uvCoordinates = model->uv(model->face_uv_indices(faceIndex)[vertIndex]); // read the UV coordinates from the .obj file
        varyingUv.set_col(vertIndex, uvCoordinates);

        Vec4f gl_Vertex = vec3fToVec4fPoint(modelCoordinates);

        varyingIntensity[vertIndex] = std::max(0.f, model->normal(model->face(faceIndex)[vertIndex]) * lightDirection); // get diffuse lighting intensity

        return Projection * ModelView * gl_Vertex;
    }
    
    virtual bool fragment(Vec3f barycentricCoodinates, BMPColor& color) {
        Vec2f uv = varyingUv * barycentricCoodinates;
        int textureX = uv.x * texture->get_width();
        int textureY = uv.y * texture->get_height();
        TGAColor textureColor = texture->get(textureX, textureY);

        float intensity = varyingIntensity * barycentricCoodinates;

        color = BMPColor(textureColor.r * intensity, textureColor.g * intensity, textureColor.b * intensity, 255);
        return false; // do not discard pixel
    }
};

Vec4f perspectiveDivide(Vec4f v) {
    for (int i = 0; i < 4; i++) {
        v[i] = v[i] / v[3];
    }
    return v;
}

/*
Current Goal: make textures work with current shader
*/
int main(int argc, char** argv) {

    // rendering data structures
    BMPImage frameBuffer(height, width, 3);
    int* zBuffer = initZBuffer(frameBuffer.getWidth(), frameBuffer.getHeight());

    // camera
    float cameraPlaneDistance = 10.0;
    Projection = projectionMatrix(cameraPlaneDistance);

    Vec3f up = Vec3f(0, 1, 0);
    Vec3f cameraPos = Vec3f(0, 0, 1);
    Vec3f lookAtPoint = Vec3f(0, 0, 0);
    Vec3f cameraDirection = Vec3f(0, 0, -1); // camera is always looking down -Z axis

    ModelView = viewMatrix(cameraPos, lookAtPoint, up);
    Viewport = viewportMatrix(width / 8.0, height / 8.0, width * 3.0 / 4.0, height * 3.0 / 4.0);

    // model/texture
    Model* model = new Model("obj/african_head.obj");
    TGAImage modelTexture = TGAImage();
    modelTexture.read_tga_file("texture/african_head_diffuse.tga");
    modelTexture.flip_vertically();

    // shader
    Shader shader = Shader(model, &modelTexture);

    // the render loop
    for (int i = 0; i < model->nfaces(); i++) {
        Vec3f screenCoordinates[3];

        // run vertex shader
        for (int j = 0; j < 3; j++) {
            Vec4f clipCoordinates = shader.vertex(i, j);

            // perspective divide
            clipCoordinates = perspectiveDivide(clipCoordinates);

            // viewport transform
            screenCoordinates[j] = vec4fToVec3f(Viewport * clipCoordinates);
        }

        // backface culling
        Vec3f faceNormal = cross(screenCoordinates[1] - screenCoordinates[0], screenCoordinates[2] - screenCoordinates[0]);
        faceNormal.normalize();

        if ((faceNormal * cameraDirection) > 0) { // negative value here means camera and surface are pointing towards each other (--> <--)
            continue;
        }

        // rasterize (calls fragment shader)
        rasterize(screenCoordinates, frameBuffer, zBuffer, shader);
    }

    // output images
    std::string imageFileName = "bitmapImage.bmp";
    frameBuffer.save(&imageFileName[0]);

    BMPImage zBufferImage = zBufferToImage(zBuffer, width, height);
    char* zBufferImageFileName = (char*)"bitmapImageZBuffer.bmp";
    zBufferImage.save(zBufferImageFileName);

    std::cout << "Image generated!!";
    delete model;
    return 0;
}