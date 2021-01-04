#define _CRT_SECURE_NO_WARNING
#define _USE_MATH_DEFINES

#include <math.h>
#include <cstdlib>
#include <iostream>
#include "bmpimage.h"
#include "line.h"
#include "model.h"
#include "triangle.h"
#include "geometry.h"
#include "shader.h"
#include "zbuffer.h"
#include <string>

const BMPColor white = BMPColor(255, 255, 255, 255);
const BMPColor grey = BMPColor(128, 128, 128, 255);
const BMPColor red = BMPColor(255, 0, 0, 255);
const BMPColor blue = BMPColor(0, 0, 255, 255);
const BMPColor green = BMPColor(0, 255, 0, 255);

const int height = 800;
const int width = 800;

Matrix rotationMatrixY(double angle) {
    angle = angle / 180 * M_PI;

    Matrix matrix;

    matrix[0][0] = std::cos(angle);
    matrix[0][2] = std::sin(angle);
    matrix[1][1] = 1.0;
    matrix[2][0] = -std::sin(angle);
    matrix[2][2] = std::cos(angle);
    matrix[3][3] = 1.0;

    return matrix;
}

int main(int argc, char** argv) {

    // rendering data structures
    BMPImage frameBuffer(height, width, 3);
    float* zBuffer = initZBuffer(frameBuffer.getWidth(), frameBuffer.getHeight());

    // camera
    Vec3f cameraDirection = Vec3f(0, 0, -1);

    float cameraPlaneDistance = 3.0;
    Matrix perspectiveMatrix;

    perspectiveMatrix[0][0] = 1.0;
    perspectiveMatrix[1][1] = 1.0;
    perspectiveMatrix[2][2] = 1.0;
    perspectiveMatrix[3][3] = 1.0;
    perspectiveMatrix[3][2] = -1.0 / cameraPlaneDistance;

    // model/texture
    Model* model = new Model("obj/african_head.obj");
    TGAImage modelTexture = TGAImage();
    modelTexture.read_tga_file("texture/african_head_diffuse.tga");
    modelTexture.flip_vertically();

    // shaders 
    ColorShader colorShader = ColorShader(white);
    FlatLightingShader flatLightingShader = FlatLightingShader(Vec3f(0, 0, -1));
    TextureShader textureShader = TextureShader(modelTexture);

    std::vector<FragmentShader*> shaders;

    //shaders.push_back(&colorShader);
    shaders.push_back(&textureShader);
    shaders.push_back(&flatLightingShader);

    // the render loop
    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        std::vector<int> faceUvIndices = model->face_uv_indices(i);

        Vec3f faceWorldCoordinates[3];
        Vec3f faceScreenCoordnates[3];
        Vec3f faceUvCoordinates[3];

        for (int j = 0; j < 3; j++) {

            faceWorldCoordinates[j] = model->vert(face[j]);
            faceUvCoordinates[j] = model->uv(faceUvIndices[j]);

            // convert to homogenous coordinates
            Vec4f faceWorldCoordinatesHomogenous;

            faceWorldCoordinatesHomogenous[0] = faceWorldCoordinates[j].x;
            faceWorldCoordinatesHomogenous[1] = faceWorldCoordinates[j].y;
            faceWorldCoordinatesHomogenous[2] = faceWorldCoordinates[j].z;
            faceWorldCoordinatesHomogenous[3] = 1.0;

            // apply perspective transform to world coordinates
            faceWorldCoordinatesHomogenous = perspectiveMatrix * faceWorldCoordinatesHomogenous;

            // apply perspective divide (divide by W) to convert out of homogenous coordinates
            faceWorldCoordinates[j][0] = faceWorldCoordinatesHomogenous[0] / faceWorldCoordinatesHomogenous[3];
            faceWorldCoordinates[j][1] = faceWorldCoordinatesHomogenous[1] / faceWorldCoordinatesHomogenous[3];
            faceWorldCoordinates[j][2] = faceWorldCoordinatesHomogenous[2] / faceWorldCoordinatesHomogenous[3];

            // vertex coordinates are floating point between -1 to 1
            // add one to make it between 0 to 2
            // calculate the fraction of 2.0 and multiply by either width/height
            int x = ((faceWorldCoordinates[j].x + 1) / 2.0) * width * 3.0 / 4.0 + width / 8.0;
            int y = ((faceWorldCoordinates[j].y + 1) / 2.0) * height * 3.0 / 4.0 + height / 8.0;

            faceScreenCoordnates[j] = Vec3f(x, y, faceWorldCoordinates[j].z);
        }

        // get normal vector of the face
        Vec3f normal = cross(faceWorldCoordinates[2] - faceWorldCoordinates[0], faceWorldCoordinates[1] - faceWorldCoordinates[0]);
        normal.normalize();

        // backface culling, skip faces that are oriented away from camera
        if ((normal * cameraDirection) <= 0) {
            continue;
        }

        rasterize(faceScreenCoordnates[0], faceScreenCoordnates[1], faceScreenCoordnates[2], faceUvCoordinates, normal, frameBuffer, zBuffer, shaders);
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