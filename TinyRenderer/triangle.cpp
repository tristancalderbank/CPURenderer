#include <vector>
#include <string>
#include "line.h"
#include "triangle.h"
#include "shader.h"
#include "geometry.h"

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

void rasterize(Triangle triangle, BMPImage& image, int* zBuffer, std::vector<FragmentShader*> shaders) {
    Vec3f p0 = triangle.vertices[0].screenCoordinates;
    Vec3f p1 = triangle.vertices[1].screenCoordinates;
    Vec3f p2 = triangle.vertices[2].screenCoordinates;

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
                    FragmentShaderInput input;

                    input.surfaceNormal = triangle.normal;

                    input.normal = Vec3f(
                        triangle.vertices[0].normal.x * barycentricCoordinates.x + triangle.vertices[1].normal.x * barycentricCoordinates.y + triangle.vertices[2].normal.x * barycentricCoordinates.z,
                        triangle.vertices[0].normal.y * barycentricCoordinates.x + triangle.vertices[1].normal.y * barycentricCoordinates.y + triangle.vertices[2].normal.y * barycentricCoordinates.z,
                        triangle.vertices[0].normal.z * barycentricCoordinates.x + triangle.vertices[1].normal.z * barycentricCoordinates.y + triangle.vertices[2].normal.z * barycentricCoordinates.z
                    );

                    input.normal.normalize();

                    input.uvCoordinates = Vec2f(
                        triangle.vertices[0].uvCoordinates.x * barycentricCoordinates.x + triangle.vertices[1].uvCoordinates.x * barycentricCoordinates.y + triangle.vertices[2].uvCoordinates.x * barycentricCoordinates.z,
                        triangle.vertices[0].uvCoordinates.y * barycentricCoordinates.x + triangle.vertices[1].uvCoordinates.y * barycentricCoordinates.y + triangle.vertices[2].uvCoordinates.y * barycentricCoordinates.z
                    );

                    for (auto shader : shaders) {
                        input.color = shader->shade(input);
                    }

                    image.set(x, y, input.color);
                    zBuffer[x + y * width] = z;
                }
            }
        }
    }
}


