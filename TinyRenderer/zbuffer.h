#pragma once

#include <limits>
#include "bmpimage.h"

int* initZBuffer(int width, int height) {
    int* zBuffer = new int[width * height];

    // init zBuffer
    for (int i = 0; i < width * height; i++) {
        zBuffer[i] = std::numeric_limits<int>::min();
    }

    return zBuffer;
}

BMPImage zBufferToImage(int* zBuffer, int width, int height) {
    BMPImage image(height, width, 3);

    // normalize to between 0-255
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            int zBufferValue = zBuffer[i + j * width];

            // ignore negative infinity values
            if (zBufferValue == std::numeric_limits<int>::min()) {
                continue;
            }

            BMPColor color = BMPColor(zBufferValue, zBufferValue, zBufferValue, 255);
            image.set(i, j, color);
        }
    }

    return image;
}