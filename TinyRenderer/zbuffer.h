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