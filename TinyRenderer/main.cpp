#define _CRT_SECURE_NO_WARNING

#include <cstdlib>
#include <iostream>
#include "bmpimage.h"
#include "line.h"
#include "model.h"
#include "triangle.h"
#include "geometry.h"

const BMPColor white = BMPColor(255, 255, 255, 255);
const BMPColor grey = BMPColor(128, 128, 128, 255);
const BMPColor red = BMPColor(255, 0, 0, 255);
const BMPColor blue = BMPColor(0, 0, 255, 255);
const BMPColor green = BMPColor(0, 255, 0, 255);

const int height = 800;
const int width = 800;

float* initZBuffer(int width, int height) {
    float* zBuffer = new float[width * height];

    // init zBuffer
    for (int i = 0; i < width * height; i++) {
        zBuffer[i] = -std::numeric_limits<float>::max();
    }

    return zBuffer;
}

BMPImage zBufferToImage(float* zBuffer, int width, int height) {
    BMPImage image(height, width, 3);

    float maxZBufferValue = -std::numeric_limits<float>::max();
    float minZBufferValue = std::numeric_limits<float>::max();

    // calculate min/max values so that we can normalize later
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float zBufferValue = zBuffer[i + j * width];
            
            // ignore negative infinity values
            if (zBufferValue == -std::numeric_limits<float>::max()) {
                continue;
            }

            maxZBufferValue = std::max(maxZBufferValue, zBufferValue);
            minZBufferValue = std::min(minZBufferValue, zBufferValue);
        }
    }

    float zBufferRange = maxZBufferValue - minZBufferValue;

    // normalize to between 0-255
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float zBufferValue = zBuffer[i + j * width];

            // ignore negative infinity values
            if (zBufferValue == -std::numeric_limits<float>::max()) {
                continue;
            }

            float zBufferValuePercent = (zBufferValue - minZBufferValue) / zBufferRange;
           
            int pixelColor = zBufferValuePercent * 255;

            BMPColor color = BMPColor(pixelColor, pixelColor, pixelColor, 255);
            image.set(i, j, color);
        }
    }

    return image;
}

int main(int argc, char** argv) {
    BMPImage image(height, width, 3);

    float* zBuffer = initZBuffer(image.getWidth(), image.getHeight());

    Model* model = new Model("obj/african_head.obj");

    Vec3f lightDirection = Vec3f(0, 0, -1);

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);

        Vec3f faceWorldCoordinates[3];
        Vec3f faceScreenCoordnates[3];

        for (int j = 0; j < 3; j++) {
            faceWorldCoordinates[j] = model->vert(face[j]);

            // just ignore Z axis (orthographic projection)
            // vertex coordinates are floating point between -1 to 1
            // add one to make it between 0 to 2
            // calculate the fraction of 2.0 and multiply by either width/height
            int x = ((faceWorldCoordinates[j].x + 1) / 2.0) * width;
            int y = ((faceWorldCoordinates[j].y + 1) / 2.0) * height;
            
            faceScreenCoordnates[j] = Vec3f(x, y, faceWorldCoordinates[j].z);
        }

        // get normal vector of the face
        Vec3f normal = cross(faceWorldCoordinates[2] - faceWorldCoordinates[0], faceWorldCoordinates[1] - faceWorldCoordinates[0]);
        normal.normalize();

        // dot the surface normal with the light direction to get intensity
        float intensity = normal * lightDirection;

        // don't bother filling completely black triangles
        if (intensity > 0) {
            BMPColor color = BMPColor(intensity * 255, intensity * 255, intensity * 255, 255);
            triangle(faceScreenCoordnates[0], faceScreenCoordnates[1], faceScreenCoordnates[2], image, zBuffer, color);
        }
    }

    BMPImage zBufferImage = zBufferToImage(zBuffer, width, height);

    char* imageFileName = (char*)"bitmapImage.bmp";
    image.save(imageFileName);

    char* zBufferImageFileName = (char*)"bitmapImageZBuffer.bmp";
    zBufferImage.save(zBufferImageFileName);

    std::cout << "Image generated!!";
    delete model;
    return 0;
}