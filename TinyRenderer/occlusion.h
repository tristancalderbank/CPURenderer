#pragma once

float maxElevationAngle(int initialX, int initialY, int dirX, int dirY, float* zBuffer, int zBufferWidth, int zBufferHeight) {

    float maxAngle = 0;
    float initialHeight = zBuffer[initialX + initialY * zBufferWidth];

    int x = initialX;
    int y = initialY;

    while (1) {
        x += dirX;
        y += dirY;

        if (x < 0 ||
            x >= zBufferWidth ||
            y < 0 ||
            y >= zBufferHeight) {
            break;
        }

        float height = zBuffer[x + y * zBufferWidth];

        float dX = x - initialX;
        float dY = y - initialY;

        float distance = std::sqrt(dX * dX + dY * dY);
        float dHeight = height - initialHeight;

        float angle = atanf(dHeight / distance);

        maxAngle = std::max(maxAngle, angle);
    }

    return maxAngle;
}