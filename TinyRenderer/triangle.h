#include "geometry.h"
#include "bmpimage.h"
#include "shader.h"

Vec3f barycentric(Vec3f p2, Vec3f p1, Vec3f p0, Vec3f point);

void rasterize(Vec3f p0, Vec3f p1, Vec3f p2, Vec3f uvCoordinates[3], Vec3f normal, BMPImage& image, float* zBuffer, std::vector<FragmentShader*> shaders);