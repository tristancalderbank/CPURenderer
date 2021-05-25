#include <vector>
#include <string>
#include "rasterize.h"
#include "shader.h"
#include "geometry.h"

void line(int x0, int y0, int x1, int y1, BMPImage& image, BMPColor color) {
    // the number of points to draw is the max of dx, dy
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    // tells whether to iterate forward/backward 
    int xDirection = x1 > x0 ? 1 : -1;
    int yDirection = y1 > y0 ? 1 : -1;

    // use the bigger diff to make sure we fill in the whole line
    if (dx > dy) {
        for (int x = x0; x != (x1 + xDirection); x += xDirection) {
            // calculate percent of the total dx, avoid dividing by zero
            float percent = dx == 0 ? 0 : (float)(std::abs(x - x0)) / (float)(dx);

            // calculate the equivalent pixel on the y-axis of the line for this percentage
            int y = y0 + (dy * percent) * yDirection;
            image.set(x, y, color);
        }
    }
    else {
        for (int y = y0; y != (y1 + yDirection); y += yDirection) {
            // calculate percent of the total dy, avoid dividing by zero
            float percent = dy == 0 ? 0 : (float)(std::abs(y - y0)) / (float)(dy);

            // calculate the equivalent pixel on the x-axis of the line for this percentage
            int x = x0 + (dx * percent) * xDirection;
            image.set(x, y, color);
        }
    }
}

void line(Vec2i p0, Vec2i p1, BMPImage& image, BMPColor color) {
    line(p0.x, p0.y, p1.x, p1.y, image, color);
}

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

bool pointOutsideImage(int x, int y, int width, int height) {
    return (x < 0 || y < 0 || x > width || y > height);
}

void rasterize(mat<3, 3, float> normalizedDeviceCoordinates, mat<3, 3, float> screenCoordinates, BMPImage& image, int* zBuffer, IShader &shader) {
    Vec3f p0 = screenCoordinates.col(0);
    Vec3f p1 = screenCoordinates.col(1);
    Vec3f p2 = screenCoordinates.col(2);

    // discard degenerate triangles
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
            if (pointOutsideImage(x, y, image.getWidth(), image.getHeight())) {
                continue;
            }

            Vec3f barycentricCoordinates = barycentric(p0, p1, p2, Vec3f(x, y, 0));

            if (pointInTriangle(barycentricCoordinates)) {
                // perform depth test using z-buffer
                // calculate interpolated z value of the current pixel
                float z = p0.z * barycentricCoordinates.x + p1.z * barycentricCoordinates.y + p2.z * barycentricCoordinates.z;

                if (z > zBuffer[x + y * width]) {
                    BMPColor color;

                    shader.fragment(normalizedDeviceCoordinates, screenCoordinates, barycentricCoordinates, color);

                    image.set(x, y, color);
                    zBuffer[x + y * width] = z;
                }
            }
        }
    }
}


