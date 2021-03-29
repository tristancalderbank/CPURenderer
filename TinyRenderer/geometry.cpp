#include "geometry.h"

Vec3f vec4fToVec3f(Vec4f v) {
    Vec3f result;

    for (int i = 0; i < 3; i++) {
        result[i] = v[i];
    }

    return result;
}

Vec4f vec3fToVec4fPoint(Vec3f v) {
    Vec4f result;

    result[0] = v.x;
    result[1] = v.y;
    result[2] = v.z;
    result[3] = 1.0;

    return result;
}

Vec4f vec3fToVec4fVector(Vec3f v) {
    Vec4f result;

    result[0] = v.x;
    result[1] = v.y;
    result[2] = v.z;
    result[3] = 0.0;

    return result;
}