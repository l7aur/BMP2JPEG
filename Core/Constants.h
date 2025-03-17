#pragma once

static constexpr int BYTE_ALIGNMENT = 32;

namespace Util {
    inline int computeAlignment(const int imageWidth) {
        int alignment = imageWidth;
        const int r = alignment % BYTE_ALIGNMENT;
        if (r == 0)
            return alignment;
        alignment += (BYTE_ALIGNMENT - r);
        return alignment;
    }
}