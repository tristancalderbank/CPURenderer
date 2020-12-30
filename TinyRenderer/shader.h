#pragma once

#include "bmpimage.h"
#include "tgaimage.h"
#include "geometry.h"

// interpolated fragment values
struct FragmentShaderInput {
    BMPColor color;
    Vec3f normal;
    Vec2f uvCoordinates;

    FragmentShaderInput() {}
};

// Acts on individual pixels of a triangle
// aka Pixel Shader
class FragmentShader {
    public:
        virtual BMPColor shade(FragmentShaderInput in) = 0;
};

// Child classes
class ColorShader : public FragmentShader {

    BMPColor color;

public:
    ColorShader(BMPColor color) : color(color) {}

    BMPColor shade(FragmentShaderInput in) {
        return color;
    }
};

class FlatLightingShader : public FragmentShader {

    Vec3f lightDirection;

    public:
        FlatLightingShader(Vec3f lightDirection) : lightDirection(lightDirection) {}

        BMPColor shade(FragmentShaderInput in) {
            float intensity = in.normal * lightDirection;
            return BMPColor(in.color.r * intensity, in.color.g * intensity, in.color.b * intensity, in.color.a);
        }
};

class TextureShader : public FragmentShader {

    TGAImage texture;

public:
    TextureShader(TGAImage texture) : texture(texture) {}

    BMPColor shade(FragmentShaderInput in) {
        int textureX = in.uvCoordinates.x * texture.get_width();
        int textureY = in.uvCoordinates.y * texture.get_height();

        TGAColor textureColor = texture.get(textureX, textureY);

        return BMPColor(textureColor.r, textureColor.g, textureColor.b, textureColor.a);
    }
};