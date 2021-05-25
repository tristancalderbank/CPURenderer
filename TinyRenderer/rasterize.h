#include "geometry.h"
#include "bmpimage.h"
#include "shader.h"

Vec3f barycentric(Vec3f p2, Vec3f p1, Vec3f p0, Vec3f point);

void rasterize(mat<3, 3, float> normalizedDeviceCoordinates, mat<3, 3, float> screenCoordinates, BMPImage& image, int* zBuffer, IShader& shader);