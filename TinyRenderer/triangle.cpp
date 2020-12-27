#include <vector>
#include "line.h"
#include "triangle.h"

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

void triangle(Vec3f p0, Vec3f p1, Vec3f p2, BMPImage& image, float* zBuffer, BMPColor color) {
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
                    image.set(x, y, color);
                    zBuffer[x + y * width] = z;
                }
            }
        }
    }
}


