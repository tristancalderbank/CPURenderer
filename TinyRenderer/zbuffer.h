#pragma once

#include <limits>
#include "bmpimage.h"

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

    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::min();

    // find max and min
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float zBufferValue = zBuffer[i + j * width];

            // ignore negative infinity values
            if (zBufferValue == -std::numeric_limits<float>::max()) {
                continue;
            }

            maxValue = std::max(maxValue, zBufferValue);
            minValue = std::min(minValue, zBufferValue);
        }
    }

    float minMaxDiff = maxValue - minValue;

    // normalize to between 0-255
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float zBufferValue = zBuffer[i + j * width];

            // ignore negative infinity values
            if (zBufferValue == -std::numeric_limits<float>::max()) {
                continue;
            }

            float intensity = (zBufferValue - minValue) / minMaxDiff;
            int colorVal = intensity * 255;

            BMPColor color = BMPColor(colorVal, colorVal, colorVal, 255);
            image.set(i, j, color);
        }
    }

    return image;
}