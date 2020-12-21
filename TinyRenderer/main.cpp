#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "bmpimage.h"
#include "line.h"
#include "model.h"

const BMPColor white = BMPColor(255, 255, 255, 255);
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
        
        // draw three lines per face to form a triangle
        for (int j = 0; j < 3; j++) {
            Vec3f vertex0 = model->vert(face[j]);
            Vec3f vertex1 = model->vert(face[(j + 1) % 3]);

            // just ignore Z axis (orthographic projection)
            // vertex coordinates are floating point between -1 to 1
            // add one to make it between 0 to 2
            // calculate the fraction of 2.0 and multiply by either width/height
            int x0 = ((vertex0.x + 1) / 2.0) * width;
            int y0 = ((vertex0.y + 1) / 2.0) * height;
            int x1 = ((vertex1.x + 1) / 2.0) * width;
            int y1 = ((vertex1.y + 1) / 2.0) * height;
            line(x0, y0, x1, y1, image, white);
        }
    }

    char* imageFileName = (char*)"bitmapImage.bmp";
    image.save(imageFileName);
    std::cout << "Image generated!!";
    delete model;
    return 0;
}