#pragma once

#include "bmpimage.h"
#include "geometry.h"

struct IShader {
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, BMPColor& color) = 0;
};