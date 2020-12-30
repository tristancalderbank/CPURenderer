#include <vector>
#include <string>
#include "line.h"
#include "triangle.h"
#include "shader.h"

bool pointInTriangle(Vec3f barycentricCoordinates) {
    return !(barycentricCoordinates.x < 0 || barycentricCoordinates.y < 0 || barycentricCoordinates.z < 0);
}

// Returns barycentric coordinates for a point with respect to a triangle
Vec3f barycentric(Vec3f a, Vec3f b, Vec3f c, Vec3f point) {
    // calculate [u v 1] vector by cross product:  [ACy ABy APy] x [ACx ABx APx]

    Vec3f xVec = Vec3f(
        c.x - a.x,
        b.x - a.x,
        a.x - point.x
    );

    Vec3f yVec = Vec3f(
        c.y - a.y,
        b.y - a.y,
        a.y - point.y
    );

    Vec3f uv = cross(yVec, xVec);

    return Vec3f(
        1.0f - (uv.x + uv.y) / uv.z,
        uv.y / uv.z,
        uv.x / uv.z
    );
}

void rasterize(Vec3f p0, Vec3f p1, Vec3f p2, Vec3f uvCoordinates[3], Vec3f normal, BMPImage& image, float* zBuffer, std::vector<FragmentShader*> shaders) {
    if (p0.y == p1.y && p0.y == p2.y) {
        return;
    }

    int width = image.getWidth();

    // compute bounding box of the triangle
    int minX = std::min(p0.x, std::min(p1.x, p2.x));
    int maxX = std::max(p0.x, std::max(p1.x, p2.x));

    int minY = std::min(p0.y, std::min(p1.y, p2.y));
    int maxY = std::max(p0.y, std::max(p1.y, p2.y));

    // iterate over the bounding box
    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            Vec3f barycentricCoordinates = barycentric(p0, p1, p2, Vec3f(x, y, 0));

            if (pointInTriangle(barycentricCoordinates)) {
                // perform depth test using z-buffer
                // calculate interpolated z value of the current pixel
                float z = p0.z * barycentricCoordinates.x + p1.z * barycentricCoordinates.y + p2.z * barycentricCoordinates.z;

                if (z > zBuffer[x + y * width]) {
                    FragmentShaderInput input;

                    input.normal = normal;

                    input.uvCoordinates = Vec2f(
                        uvCoordinates[0].x * barycentricCoordinates.x + uvCoordinates[1].x * barycentricCoordinates.y + uvCoordinates[2].x * barycentricCoordinates.z,
                        uvCoordinates[0].y * barycentricCoordinates.x + uvCoordinates[1].y * barycentricCoordinates.y + uvCoordinates[2].y * barycentricCoordinates.z
                    );

                    for (auto shader : shaders) {
                        input.color = shader->shade(input);
                    }

                    image.set(x, y, input.color);
                    zBuffer[x + y * width] = z;

                    //// output the current image
                    //if ((frame > 11000 && frame < 12000 && frame % 2 == 0) || (frame % 100 == 0)) {
                    //    std::string imageFileName = "video/";
                    //    imageFileName += std::to_string(frame);
                    //    imageFileName += ".bmp";
                    //    image.save(&imageFileName[0]);
                    //}
                    //frame++;
                }
            }
        }
    }
}


