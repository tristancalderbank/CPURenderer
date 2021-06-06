#pragma once

int signum(float num) {
    if (num > 0.00001)
        return 1;
    if (num < -0.00001)
        return -1;
    return 0;
}