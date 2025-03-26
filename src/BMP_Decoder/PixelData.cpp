#include "PixelData.h"

#include <unistd.h>

PixelData::PixelData(const size_t s)
    : dataSize(s)
{
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
        std::cerr << "Failed while initializing the color table!\n";
        return -1;
    }
    if (setEncoding(bitCount, compression) < 0)
    {
        std::cerr << "Unhandled pixel data encoding! Aborting!\n";
        return -1;
    }
    if (lseek(fd, offset, SEEK_SET) < 0) {
        std::cerr << "Failed to reach pixel data starting point!\n";
        return -1;
    }
    dataSize = read(fd, data, dataSize);
    if (dataSize < 0)
    {
        std::cerr << "Error while reading \'pixel data\'\n";
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
            std::cerr << "Unknown encoding for no compression pixel data representation: " << encoding << " bits\n";
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
    std::cerr << "Unknown encoding for pixel data representation: " << encoding << " bits\n";
    return -1;
}

int PixelData::initColorTable(const int fd, const uint16_t bitCount)
{
    if (bitCount > 8)
    {
        std::cout << "INFO: There is no color table for this format!\n";
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
    // colorTab->print();
}

const uint32_t *PixelData::getAndFormatData(const int imageWidth, const int imageHeight) const
{
    switch (encoding)
    {
    case C8:
        return format_C8(imageWidth, imageHeight);
    default:
        break;
    }
    std::cerr << "Unknown format! Unable to format pixel data!\n";
    return nullptr;
}

const uint32_t *PixelData::format_C8(const int imageWidth, const int imageHeight) const {
    const int rowSize = (imageWidth + 3) & ~3;
    const int NUMBER_OF_PIXELS = imageHeight * imageWidth;
    auto *pixels = new uint32_t[NUMBER_OF_PIXELS];

    for (int y = 0; y < imageHeight; ++y) {
        for (int x = 0; x < imageWidth; ++x) {
            const Pixel p = colorTable->at(data[(imageHeight - 1 - y) * rowSize + x]);
            pixels[y * imageWidth + x] = (p.r << 24) | (p.g << 16) | (p.b << 8) | p.a;
        }
    }
    return pixels;
}

