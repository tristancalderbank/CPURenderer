#include "geometry.h"
#include "bmpimage.h"

Vec3f barycentric(Vec3f p2, Vec3f p1, Vec3f p0, Vec3f point);

void rasterize(Vec3f t0, Vec3f t1, Vec3f t2, BMPImage& image, float* zBuffer, BMPColor color);