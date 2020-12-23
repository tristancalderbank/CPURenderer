#include <vector>
#include "line.h"
#include "triangle.h"

void triangle(Vec2i p0, Vec2i p1, Vec2i p2, BMPImage& image, BMPColor color) {
    // idea draw a line from p0 to every point on the line between p1 and p2

    std::vector<Vec2i> p1_p2_line_points = line_points(p1.x, p1.y, p2.x, p2.y);

    for (const Vec2i& p : p1_p2_line_points) {
        line(p0, p, image, color);
    }
}
