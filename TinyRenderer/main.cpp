#define _CRT_SECURE_NO_WARNING

#include <cstdlib>
#include <iostream>
#include "bmpimage.h"
#include "line.h"
#include "model.h"
#include "triangle.h"

const BMPColor white = BMPColor(255, 255, 255, 255);
const BMPColor grey = BMPColor(128, 128, 128, 255);
const BMPColor red = BMPColor(255, 0, 0, 255);
const BMPColor blue = BMPColor(0, 0, 255, 255);
const BMPColor green = BMPColor(0, 255, 0, 255);

const int height = 800;
const int width = 800;

int main(int argc, char** argv) {
    BMPImage image(height, width, 3);
    
    Model* model = new Model("obj/african_head.obj");

    Vec3f lightDirection = Vec3f(0, 0, -1);

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);

        Vec3f faceWorldCoordinates[3];
        Vec2i faceScreenCoordnates[3];

        for (int j = 0; j < 3; j++) {
            faceWorldCoordinates[j] = model->vert(face[j]);

            // just ignore Z axis (orthographic projection)
            // vertex coordinates are floating point between -1 to 1
            // add one to make it between 0 to 2
            // calculate the fraction of 2.0 and multiply by either width/height
            int x = ((faceWorldCoordinates[j].x + 1) / 2.0) * width;
            int y = ((faceWorldCoordinates[j].y + 1) / 2.0) * height;
            
            faceScreenCoordnates[j] = Vec2i(x, y);
        }

        // get normal vector of the face
        Vec3f normal = (faceWorldCoordinates[2] - faceWorldCoordinates[0]) ^ (faceWorldCoordinates[1] - faceWorldCoordinates[0]);
        normal.normalize();

        // dot the surface normal with the light direction to get intensity
        float intensity = normal * lightDirection;

        // don't bother filling completely black triangles
        if (intensity > 0) {
            BMPColor color = BMPColor(intensity * 255, intensity * 255, intensity * 255, 255);
            triangle(faceScreenCoordnates[0], faceScreenCoordnates[1], faceScreenCoordnates[2], image, color);
        }
    }

    char* imageFileName = (char*)"bitmapImage.bmp";
    image.save(imageFileName);
    std::cout << "Image generated!!";
    delete model;
    return 0;
}