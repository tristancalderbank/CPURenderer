#pragma once

#include "bmpimage.h"
#include "geometry.h"

struct FragmentShaderInput {
    Vec3f normal;

    FragmentShaderInput(Vec3f normal) : normal(normal) {
    }
};

// Acts on individual pixels of a triangle
// aka Pixel Shader
class FragmentShader {
    public:
        virtual BMPColor shade(FragmentShaderInput in) = 0;
};

// Derived classes
class FlatLightingShader : public FragmentShader {

    Vec3f lightDirection;

    public:
        FlatLightingShader(Vec3f lightDirection) : lightDirection(lightDirection) {}

        BMPColor shade(FragmentShaderInput in) {
            float intensity = in.normal * lightDirection;
            return BMPColor(intensity * 255, intensity * 255, intensity * 255, 255);
        }
};