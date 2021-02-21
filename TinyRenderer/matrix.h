#pragma once
#define _USE_MATH_DEFINES

#include "geometry.h"

const int zBufferDepth = 255;

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

Matrix identityMatrix() {
    Matrix matrix;

    for (int i = 0; i < 4; i++) {
        matrix[i][i] = 1.0;
    }

    return matrix;
}

Matrix viewMatrix(Vec3f cameraPos, Vec3f lookAtPoint, Vec3f up) {
    Vec3f z = (cameraPos - lookAtPoint).normalize(); // z should point towards the camera
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();

    Matrix mInverse = identityMatrix(); // rotates model to lookAtPoint frame orientation

    mInverse[0][0] = x[0]; // x as a row vector
    mInverse[0][1] = x[1];
    mInverse[0][2] = x[2];

    mInverse[1][0] = y[0]; // y as a row vector
    mInverse[1][1] = y[1];
    mInverse[1][2] = y[2];

    mInverse[2][0] = z[0]; // z as a row vector
    mInverse[2][1] = z[1];
    mInverse[2][2] = z[2];

    Matrix translation = identityMatrix(); // shifts model to origin by the difference in origin between the two frames

    translation[3][0] = -lookAtPoint[0];
    translation[3][1] = -lookAtPoint[1];
    translation[3][2] = -lookAtPoint[2];

    return mInverse * translation;
}

Matrix projectionMatrix(float cameraPlaneDistance) {
    Matrix projectionMatrix = identityMatrix();
    projectionMatrix[3][2] = -1.0 / cameraPlaneDistance;

    return projectionMatrix;
}

// vertex coordinates are floating point between -1 to 1
// add one to make it between 0 to 2
// calculate the fraction of 2.0 and multiply by either width/height
// (x,y) here let us shift the resulting image within the viewport
// using these only makes sense if we use (w, h) less than the real viewport (w, h), otherwise we'd be shifting it off screen
Matrix viewportMatrix(int x, int y, int w, int h) {
    Matrix viewport = identityMatrix();

    viewport[0][0] = w / 2.0;
    viewport[1][1] = h / 2.0;
    viewport[2][2] = zBufferDepth / 2.0;

    viewport[0][3] = w / 2.0 + x;
    viewport[1][3] = h / 2.0 + y;
    viewport[2][3] = zBufferDepth / 2.0;

    return viewport;
}