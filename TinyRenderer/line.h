#pragma once

#include <vector>
#include "bmpimage.h"
#include "geometry.h"

void line(int x0, int y0, int x1, int y1, BMPImage& image, BMPColor color);
void line(Vec2i p0, Vec2i p1, BMPImage& image, BMPColor color);