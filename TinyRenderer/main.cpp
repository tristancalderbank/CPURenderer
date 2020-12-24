#define _CRT_SECURE_NO_WARNINGS

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

    for (int i = 0; i < model->nfaces(); i++) {
        std::vector<int> face = model->face(i);

        Vec2i faceScreenCoordnates[3];

        for (int j = 0; j < 3; j++) {
            Vec3f faceWorldCoordinates = model->vert(face[j]);

            // just ignore Z axis (orthographic projection)
            // vertex coordinates are floating point between -1 to 1
            // add one to make it between 0 to 2
            // calculate the fraction of 2.0 and multiply by either width/height
            int x = ((faceWorldCoordinates.x + 1) / 2.0) * width;
            int y = ((faceWorldCoordinates.y + 1) / 2.0) * height;
            
            faceScreenCoordnates[j] = Vec2i(x, y);
        }

        BMPColor color = BMPColor(std::rand() % 255, std::rand() % 255, std::rand() % 255, 255);
        triangle(faceScreenCoordnates[0], faceScreenCoordnates[1], faceScreenCoordnates[2], image, color);
    }

    char* imageFileName = (char*)"bitmapImage.bmp";
    image.save(imageFileName);
    std::cout << "Image generated!!";
    delete model;
    return 0;
}