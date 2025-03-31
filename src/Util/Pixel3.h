#pragma once
#include <cstdint>
#include <ostream>

namespace Util {
    struct Pixel3 {
        union { uint8_t r, y; };
        union { uint8_t g, cb; };
        union { uint8_t b, cr; };
        union { uint8_t a, u; };
        explicit Pixel3(const uint8_t x = 0xFF, const uint8_t y = 0xFF, const uint8_t z = 0xFF, const uint8_t t = 0xFF)
            : r(x), g(y), b(z), a(t) {}
        explicit Pixel3(const uint32_t p) {
            r = p >> 24;
            g = p >> 16;
            b = p >> 8;
            a = p;
        }
        [[nodiscard]] uint32_t to_uint32() const {
            return (r << 24) | (g << 16) | (b << 8) | a;
        }

        friend std::ostream& operator<<(std::ostream& out, const Pixel3& p) {
            out << "(" << static_cast<int>(p.r)
                << ", " << static_cast<int>(p.g)
                << ", " << static_cast<int>(p.b)
                << ", " << static_cast<int>(p.a)
                << ")";
            return out;
        }
    };
}