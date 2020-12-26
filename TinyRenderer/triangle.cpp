#include <vector>
#include "line.h"
#include "triangle.h"

bool pointInTriangle(Vec2i point, Vec2i p0, Vec2i p1, Vec2i p2) {
    // Based on: https://stackoverflow.com/a/14382692/7401903
    // To see how its derived: https://en.wikipedia.org/wiki/Barycentric_coordinate_system#Conversion_between_barycentric_and_Cartesian_coordinates

    // calculate area (easy way to get determinant for the barycentric calculation later)
    float area = 0.5 * (-p1.y * p2.x + p0.y * (-p1.x + p2.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y);

    // calculate barycentric coordinates of the point with respect to the triangle
    float s = 1.0 / (2.0 * area) * (p0.y * p2.x - p0.x * p2.y + (p2.y - p0.y) * point.x + (p0.x - p2.x) * point.y);

    if (s < 0) {
        return false;
    }
    
    float t = 1.0 / (2.0 * area) * (p0.x * p1.y - p0.y * p1.x + (p0.y - p1.y) * point.x + (p1.x - p0.x) * point.y);

    if (t < 0) {
        return false;
    }

    // check 3rd coordinate
    if ((1.0 - s - t) < 0) {
        return false;
    }

    return true;
}

void triangle(Vec2i p0, Vec2i p1, Vec2i p2, BMPImage& image, BMPColor color) {
    if (p0.y == p1.y && p0.y == p2.y) {
        return;
    }

    // compute bounding box of the triangle
    int minX = std::min(p0.x, std::min(p1.x, p2.x));
    int maxX = std::max(p0.x, std::max(p1.x, p2.x));

    int minY = std::min(p0.y, std::min(p1.y, p2.y));
    int maxY = std::max(p0.y, std::max(p1.y, p2.y));

    // iterate over the bounding box

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            if (pointInTriangle(Vec2i(x, y), p0, p1, p2)) {
                image.set(x, y, color);
            }
        }
    }
   
    line(p0, p1, image, color);
    line(p1, p2, image, color);
    line(p2, p0, image, color);
}


