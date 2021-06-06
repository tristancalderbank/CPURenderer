#pragma once

#include "bmpimage.h"
#include "geometry.h"

struct IShader {
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(int fragX, int fragY, mat<3, 3, float> normalizedDeviceCoordinates, mat<3, 3, float> screenCoordinates, Vec3f bar, BMPColor& color) = 0;
};