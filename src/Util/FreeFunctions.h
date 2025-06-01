#pragma once

#include <vector>

#include "ImageRenderer.h"
#include "Pixel4.h"

namespace Util {
    inline Pixel4 rgbToYCrCb(const Pixel4 rgbPix) {
        const auto r = static_cast<float>(rgbPix.r);
        const auto g = static_cast<float>(rgbPix.g);
        const auto b = static_cast<float>(rgbPix.b);

        const float y = .299f * r + .587f * g + .114f * b;
        const float cb = 128.0f - .168736f * r - .331264f * g + .5f * b;
        const float cr = 128.0f + .5f * r - .418688f * g - .081312f * b;

        return Pixel4{static_cast<uint8_t>(y), static_cast<uint8_t>(cb), static_cast<uint8_t>(cr)};
    }

     inline void renderPixels(const uint32_t * pixels, const int width, const int height) {
        ImageRenderer renderer{};
        if (renderer.init() < 0)
        {
            std::cerr << "[ERROR] Renderer failed to init, exiting!\n";
            return;
        }
        if (renderer.initTexture(width, height, pixels) < 0)
        {
            std::cerr << "[ERROR] Renderer failed to initialize texture!\n";
            return;
        }
        renderer.runGameLoop();
    }

    inline std::pair<int, int> computeTLHCCoordinates(const int WINDOW_WIDTH, const int WINDOW_HEIGHT,
                                                      const int textureWidth, const int textureHeight) {
        const std::pair windowCenter = { WINDOW_HEIGHT / 2, WINDOW_WIDTH / 2 };
        const std::pair textureCenter = { textureHeight / 2, textureWidth / 2 };
        return {windowCenter.first - textureCenter.first, windowCenter.second - textureCenter.second};
    }

    inline std::vector<uint8_t> convertIntToTwoBytes(const int n) {
        return { static_cast<uint8_t>((n >> 8) & 0xFF), static_cast<uint8_t>(n & 0xFF) };
    }

    inline bool isPowerOf2(const int x) {
        return x && (!(x & (x - 1)));
    }

    inline std::vector<Pixel4> convertToPixelArray(const uint32_t* pixels, const int width, const int height) {
        std::vector<Pixel4> convertedPixels;

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                convertedPixels.emplace_back(pixels[(height - 1 - i) * width + (width - 1 - j)]);
                // convertedPixels.emplace_back(pixels[i * height + j]);
            }
        }

        return convertedPixels;
    }
}
