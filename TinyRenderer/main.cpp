#define _CRT_SECURE_NO_WARNING
#define _USE_MATH_DEFINES

#include <math.h>
#include <cstdlib>
#include <iostream>
#include "geometry.h"
#include "bmpimage.h"
#include "line.h"
#include "model.h"
#include "triangle.h"
#include "shader.h"
#include "zbuffer.h"
#include <string>

const BMPColor white = BMPColor(255, 255, 255, 255);
const BMPColor grey = BMPColor(128, 128, 128, 255);
const BMPColor red = BMPColor(255, 0, 0, 255);
const BMPColor blue = BMPColor(0, 0, 255, 255);
const BMPColor green = BMPColor(0, 255, 0, 255);

const int height = 800;
const int width = 800;
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

/*
  Notes:

  * positive z-axis is towards viewer by convention.

  Flat lighting shader makes sense, light is facing -1 z which is into the screen, gouraud one doesn't make sense

*/
int main(int argc, char** argv) {

    // rendering data structures
    BMPImage frameBuffer(height, width, 3);
    int* zBuffer = initZBuffer(frameBuffer.getWidth(), frameBuffer.getHeight());

    // camera
    float cameraPlaneDistance = 10.0;
    Matrix perspectiveMatrix = identityMatrix();
    perspectiveMatrix[3][2] = -1.0 / cameraPlaneDistance;

    Vec3f up = Vec3f(0, 1, 0);
    Vec3f cameraPos = Vec3f(1, 1, 1);
    Vec3f lookAtPoint = Vec3f(0, 0, 0);
    Vec3f cameraDirection = Vec3f(0, 0, -1); // camera is always looking down -Z axis

    Matrix view = viewMatrix(cameraPos, lookAtPoint, up);
    Matrix viewport = viewportMatrix(width / 8.0, height / 8.0, width * 3.0 / 4.0, height * 3.0 / 4.0);

    // model/texture
    Matrix modelMatrix = identityMatrix();

    Model* model = new Model("obj/african_head.obj");
    TGAImage modelTexture = TGAImage();
    modelTexture.read_tga_file("texture/african_head_diffuse.tga");
    modelTexture.flip_vertically();

    // shaders 
    ColorShader colorShader = ColorShader(white);
    FlatLightingShader flatLightingShader = FlatLightingShader(Vec3f(0, 0, 1));
    GouraudShader gouraudShader = GouraudShader(Vec3f(0, -1, 0));
    TextureShader textureShader = TextureShader(modelTexture);

    std::vector<FragmentShader*> shaders;

    shaders.push_back(&colorShader);
    shaders.push_back(&textureShader);
    // shaders.push_back(&flatLightingShader);
    // shaders.push_back(&gouraudShader);

    // the render loop
    for (int i = 0; i < model->nfaces(); i++) {
        Triangle triangle;

        std::vector<int> faceVertexIndices = model->face(i);
        std::vector<int> faceUvIndices = model->face_uv_indices(i);

        for (int j = 0; j < 3; j++) {
            Vertex vertex;

            vertex.uvCoordinates = model->uv(faceUvIndices[j]);
            vertex.normal = model->normal(faceVertexIndices[j]);

            Vec3f modelCoordinates = model->vert(faceVertexIndices[j]);
            Vec4f modelCoordinatesHomogenous;
            modelCoordinatesHomogenous[0] = modelCoordinates.x;
            modelCoordinatesHomogenous[1] = modelCoordinates.y;
            modelCoordinatesHomogenous[2] = modelCoordinates.z;
            modelCoordinatesHomogenous[3] = 1.0;

            Vec4f worldCoordinatesHomogenous = modelMatrix * modelCoordinatesHomogenous;

            // apply perspective transform to world coordinates
            Vec4f clipCoordinatesHomogenous = perspectiveMatrix * view * worldCoordinatesHomogenous;

            clipCoordinatesHomogenous[0] = clipCoordinatesHomogenous[0] / clipCoordinatesHomogenous[3];
            clipCoordinatesHomogenous[1] = clipCoordinatesHomogenous[1] / clipCoordinatesHomogenous[3];
            clipCoordinatesHomogenous[2] = clipCoordinatesHomogenous[2] / clipCoordinatesHomogenous[3];
            clipCoordinatesHomogenous[3] = clipCoordinatesHomogenous[3] / clipCoordinatesHomogenous[3];

            // apply perspective divide (divide by W) to convert out of homogenous coordinates
            vertex.clipCoordinates[0] = clipCoordinatesHomogenous[0];
            vertex.clipCoordinates[1] = clipCoordinatesHomogenous[1];
            vertex.clipCoordinates[2] = clipCoordinatesHomogenous[2];

            Vec4f screen = viewport * clipCoordinatesHomogenous;

            vertex.screenCoordinates = Vec3f(screen[0], screen[1], screen[2]);

            triangle.vertices[j] = vertex;
        }

        // get normal vector of the face
        triangle.normal = cross(triangle.vertices[1].clipCoordinates - triangle.vertices[0].clipCoordinates, triangle.vertices[2].clipCoordinates - triangle.vertices[0].clipCoordinates);
        triangle.normal.normalize();

        // backface culling, skip faces that are oriented away from camera
        if ((triangle.normal * cameraDirection) > 0) { // negative value here means camera and surface are pointing towards each other
            continue;
        }

        rasterize(triangle, frameBuffer, zBuffer, shaders);
    }

    // output images
    std::string imageFileName = "bitmapImage.bmp";
    frameBuffer.save(&imageFileName[0]);

    BMPImage zBufferImage = zBufferToImage(zBuffer, width, height);
    char* zBufferImageFileName = (char*)"bitmapImageZBuffer.bmp";
    zBufferImage.save(zBufferImageFileName);

    std::cout << "Image generated!!";
    delete model;
    return 0;
}