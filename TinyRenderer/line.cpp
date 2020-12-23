#include <cstdlib>
#include <vector>
#include "bmpimage.h"
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

// Get an array of points that form a line
std::vector<Vec2i> line_points(int x0, int y0, int x1, int y1) {
	// array of points to return
	std::vector<Vec2i> points;

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
			
			// store the point
			points.push_back(Vec2i(x, y));
		}
	}
	else {
		for (int y = y0; y != (y1 + yDirection); y += yDirection) {
			// calculate percent of the total dy, avoid dividing by zero
			float percent = dy == 0 ? 0 : (float)(std::abs(y - y0)) / (float)(dy);

			// calculate the equivalent pixel on the x-axis of the line for this percentage
			int x = x0 + (dx * percent) * xDirection;

			// store the point
			points.push_back(Vec2i(x, y));
		}
	}

	return points;
}

void line(Vec2i p0, Vec2i p1, BMPImage& image, BMPColor color) {
	line(p0.x, p0.y, p1.x, p1.y, image, color);
}