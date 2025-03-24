#pragma once

#include <cstdint>

#include "ColorTable.h"

enum PIXEL_DATA_ENCODING
{
    BW1, // black-white, 1 bit
    C4,  // color images, 4 bits
    C8,  // color images, 8 bits
    C16, // color images, 16 bits
    C24, // color images 24 bits
    C4COMPRESSED,
    C8COMPRESSED
};

class PixelData
{
public:
    uint8_t *data{nullptr};
    ColorTable *colorTable{nullptr};
    size_t dataSize{0};
    PIXEL_DATA_ENCODING encoding{BW1};
    float indexStep{0.0f};

    explicit PixelData(size_t s);
    ~PixelData();
    int initFrom(int fd, int unsigned offset, uint16_t bitCount, uint32_t compression);
    void print() const;
    [[nodiscard]] size_t getSize() const { return dataSize; }
    [[nodiscard]] const uint32_t *getAndFormatData(int imageWidth, int imageHeight) const;

private:
    int setEncoding(uint16_t bitCount, uint32_t compression);
    int initColorTable(int fd, uint16_t bitCount);
    [[nodiscard]] const uint32_t* format_C8(int imageWidth, int imageHeight) const ;
};