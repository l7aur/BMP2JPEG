#pragma once

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <string>

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
    ColorTable *colorTab{nullptr};
    size_t dataSize{0};
    PIXEL_DATA_ENCODING encoding{BW1};
    float indexStep{0.0f};

    PixelData(const size_t s);
    ~PixelData();
    int initFrom(const int fd, const uint16_t bitCount, const uint32_t compression);
    void print() const;
    size_t getSize() const { return dataSize; }
    const uint32_t *getAndFormatData(const int imageWidth, const int imageHeight, const int byteWidth) const;

private:
    int setEncoding(const uint16_t bitCount, const uint32_t compression);
    int initColorTable(const int fd, const uint16_t bitCount);
    const uint32_t* format_C8(const int imageWidth, const int imageHeight, const int byteWidth) const;
};