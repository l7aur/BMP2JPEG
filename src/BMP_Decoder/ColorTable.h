#pragma once

#include "../Util/Pixel3.h"

#include <cstdint>
#include <cstddef>

/**
 * Object storing a BMP color table.
 */
class ColorTable
{
public:
    uint8_t *data{nullptr};
    size_t tableSize{0};
    explicit ColorTable(const size_t size)
        : tableSize(size)
    {
        data = new uint8_t[size];
    }

    [[nodiscard]] int initFrom(int fd) const;
    [[nodiscard]] Util::Pixel3 at(uint8_t i) const;
    void print() const;
};