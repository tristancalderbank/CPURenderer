#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "bmpimage.h"
#include "line.h"
#include "model.h"
#include "triangle.h"

const BMPColor white = BMPColor(255, 255, 255, 255);
const BMPColor red = BMPColor(255, 0, 0, 255);
const BMPColor blue = BMPColor(0, 0, 255, 255);
const BMPColor green = BMPColor(0, 255, 0, 255);

const int height = 200;
const int width = 200;

int main(int argc, char** argv) {
    BMPImage image(height, width, 3);
    
    Model* model = new Model("obj/african_head.obj");

    // triangles
    Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
    Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
    Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

    triangle(t0[0], t0[1], t0[2], image, red);
    triangle(t1[0], t1[1], t1[2], image, white);
    triangle(t2[0], t2[1], t2[2], image, green);

    char* imageFileName = (char*)"bitmapImage.bmp";
    image.save(imageFileName);
    std::cout << "Image generated!!";
    delete model;
    return 0;
}