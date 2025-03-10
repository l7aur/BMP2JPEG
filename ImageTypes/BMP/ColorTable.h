#pragma once

#include <cstdint>
#include <cstddef>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

struct Pixel
{
    uint8_t r, g, b;
    union
    {
        uint8_t a, u;
    };
    explicit Pixel(uint8_t x = 0x00, uint8_t y = 0x00, uint8_t z = 0x00, uint8_t t = 0xFF)
        : r(x), g(y), b(z), a(t) {}
    friend std::ostream& operator<<(std::ostream& out, const Pixel& p) {
        out << "R: " << static_cast<int>(p.r)
            << " G: " << static_cast<int>(p.g)
            << " B: " << static_cast<int>(p.b)
            << " A|U: " << static_cast<int>(p.a);
        return out;
    }
};

/**
 * Object storing a BMP color table.
 */
class ColorTable
{
public:
    uint8_t *data{nullptr};
    size_t tableSize{0};
    ColorTable(size_t size) : tableSize(size)
    {
        data = new uint8_t[size];
    }

    int initFrom(const int fd);

    Pixel at(size_t i) const;

    void print();
};