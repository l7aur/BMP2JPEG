#include "PixelData.h"
#include "../Util/Pixel3.h"

#include <iostream>
#include <unistd.h>

PixelData::PixelData(size_t s) {
    data = new uint8_t[s];
}

PixelData::~PixelData()
{
    delete colorTable;
    delete[] data;
}

int PixelData::initFrom(const int fd, const unsigned int offset, const uint16_t bitCount, const uint32_t compression)
{
    if (initColorTable(fd, bitCount) < 0)
    {
        std::cerr << "[ERROR] Failed while initializing the color table!\n";
        return -1;
    }
    if (setEncoding(bitCount, compression) < 0)
    {
        std::cerr << "[ERROR] Unhandled pixel data encoding! Aborting!\n";
        return -1;
    }
    if (lseek(fd, offset, SEEK_SET) < 0) {
        std::cerr << "[ERROR] Failed to reach pixel data starting point!\n";
        return -1;
    }
    dataSize = read(fd, data, MAXIMUM_PXD_SIZE_IN_BYTES);
    if (dataSize < 0)
    {
        std::cerr << "[ERROR] Could not read \'pixel data\'\n";
        return -1;
    }
    return 0;
}

int PixelData::setEncoding(const uint16_t bitCount, const uint32_t compression)
{
    if (compression == 0)
    {
        switch (bitCount)
        {
        case 1:
            encoding = BW1;
            return 0;
        case 4:
            encoding = C4;
            return 0;
        case 8:
            encoding = C8;
            return 0;
        case 16:
            encoding = C16;
            return 0;
        case 24:
            encoding = C24;
            return 0;
        default:
            std::cerr << "[ERROR] Unknown encoding for no compression pixel data representation: " << encoding << " bits\n";
            return -1;
        }
    }
    if (compression == 2 && bitCount == 4)
    {
        encoding = C4COMPRESSED;
        return 0;
    }
    if (compression == 1 && bitCount == 8)
    {
        encoding = C8COMPRESSED;
        return 0;
    }
    std::cerr << "[ERROR] Unknown encoding for pixel data representation: " << encoding << " bits\n";
    return -1;
}

int PixelData::initColorTable(const int fd, const uint16_t bitCount)
{
    if (bitCount > 8)
    {
        std::cout << "[INFO]  There is no color table for this .bmp format!\n";
        return 0;
    }
    colorTable = new ColorTable{static_cast<size_t>(1 << (bitCount + 2))};
    return colorTable->initFrom(fd);
}

void PixelData::print() const
{
    std::cout << "===============PIXEL-DATA================\n";
    std::cout << "Size of pixeldata: " << dataSize << '\n';
    std::cout << "Type of pixeldata: " << encoding << '\n';
    std::cout << "=============END-PIXEL-DATA==============\n";
    colorTable->print();
}

const uint32_t *PixelData::getAndFormatData(const int imageWidth, const int imageHeight) const {
    switch (encoding)
    {
        case C4:
            return format_C4(imageWidth, imageHeight);
        case C8:
            return format_C8(imageWidth, imageHeight);
        case C24:
            return format_C24(imageWidth, imageHeight);
        default:
            std::cerr << "[ERROR] Unknown format! Unable to format pixel data!\n";
        break;
    }
    return nullptr;
}

const uint32_t * PixelData::format_C4(const int imageWidth, const int imageHeight) const {
    const int rowSize = (imageWidth + 3) & ~3;
    const int NUMBER_OF_PIXELS = imageHeight * imageWidth * 2;
    auto *pixels = new uint32_t[NUMBER_OF_PIXELS];
    std::cout << imageWidth <<' ' <<imageHeight << '\n';

    int index = 0;
    for (int y = 0; y < imageHeight; ++y)
        for (int x = 0; x < imageWidth; ++x) {
            const Util::Pixel3 higherPixel = colorTable->at((data[(imageHeight - 1 - y) * rowSize + x] & 0xF0) >> 4);
            const Util::Pixel3 lowerPixel = colorTable->at(data[(imageHeight - 1 - y) * rowSize + x] & 0x0F);

            pixels[index++] = (higherPixel.b << 24) | (higherPixel.g << 16) | (higherPixel.r << 8) | higherPixel.a;
            pixels[index++] = (lowerPixel.b << 24) | (lowerPixel.g << 16) | (lowerPixel.r << 8) | lowerPixel.a;
        }
    return pixels;
}

const uint32_t *PixelData::format_C8(const int imageWidth, const int imageHeight) const {
    const int rowSize = (imageWidth + 3) & ~3;
    const int NUMBER_OF_PIXELS = imageHeight * imageWidth;
    auto *pixels = new uint32_t[NUMBER_OF_PIXELS];

    for (int y = 0; y < imageHeight; ++y)
        for (int x = 0; x < imageWidth; ++x) {
            const Util::Pixel3 p = colorTable->at(data[(imageHeight - 1 - y) * rowSize + x]);
            pixels[y * imageWidth + x] = (p.b << 24) | (p.g << 16) | (p.r << 8) | p.a;
        }
    return pixels;
}

const uint32_t * PixelData::format_C24(const int imageWidth, const int imageHeight) const {
    const int rowSize = (imageWidth * 3 + 3) & ~3;
    const int NUMBER_OF_PIXELS = imageHeight * imageWidth;
    auto *pixels = new uint32_t[NUMBER_OF_PIXELS];

    for (int y = 0; y < imageHeight; ++y)
        for (int x = 0; x < imageWidth; ++x) {
            const int dataIndex = ((imageHeight - y - 1) * rowSize + 3 * x);
            const Util::Pixel3 p{data[dataIndex], data[dataIndex + 1], data[dataIndex + 2]};
            pixels[y * imageWidth + x] = (p.b << 24) | (p.g << 16) | (p.r << 8) | p.a;
        }
    return pixels;
}

