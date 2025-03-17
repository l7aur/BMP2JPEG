#include "PixelData.h"

#include <unistd.h>

#include "../../Core/Constants.h"

PixelData::PixelData(const size_t s)
    : dataSize(s)
{
    data = new uint8_t[s];
}

PixelData::~PixelData()
{
    delete colorTab;
    delete[] data;
}

int PixelData::initFrom(const int fd, const uint16_t bitCount, const uint32_t compression)
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
    else if (compression == 2 && bitCount == 4)
    {
        encoding = C4COMPRESSED;
        return 0;
    }
    else if (compression == 1 && bitCount == 8)
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
    colorTab = new ColorTable{static_cast<size_t>(1 << (bitCount + 2))};
    return colorTab->initFrom(fd);
}

void PixelData::print() const
{
    std::cout << "===============PIXEL-DATA================\n";
    std::cout << "Size of pixeldata: " << dataSize << '\n';
    std::cout << "Type of pixeldata: " << encoding << '\n';
    std::cout << "=============END-PIXEL-DATA==============\n";
    colorTab->print();
}

const uint32_t *PixelData::getAndFormatData(const int imageWidth, const int imageHeight) const
{
    switch (encoding)
    {
    case C8:
        return format_C8(imageWidth, imageHeight, Util::computeAlignment(imageWidth));
    default:
        break;
    }
    std::cerr << "Unknown format! Unable to format pixel data!\n";
    return nullptr;
}

const uint32_t *PixelData::format_C8(const int imageWidth, const int imageHeight, const int alignment) {
    const int NUMBER_OF_PIXELS = alignment * imageHeight;
    auto *pixels = new uint32_t[NUMBER_OF_PIXELS];
    unsigned int columnCounter = 0;
    bool addPadding = true;

    for (int x = imageHeight - 1; x >= 0; --x) {
        int y = alignment - 1;
        while (y >= imageWidth) {
            pixels[x * alignment + y] = 0xff'ff'00'ff;
            y--;
        }
        while (y >= 0) {
            pixels[x * alignment + y] = 0x00'ff'00'ff;
            y--;
        }
    }
    // for (int i = NUMBER_OF_PIXELS - 1; i >= 0; --i, ++columnCounter) {
    //     if (!addPadding) {
    //         // const Pixel p = colorTab->at(data[]);
    //         pixels[i] = 0xff'ff'ff'ff; // (p.r << 24) | (p.g << 16) | (p.b << 8) | p.a;
    //         if (columnCounter == (alignment - 1)) {
    //             columnCounter = 0;
    //             addPadding = true;
    //         }
    //     }
    //     else {
    //         pixels[i] = 0xff'00'00'00;
    //         if (columnCounter == (alignment - imageWidth)) {
    //             addPadding = false;
    //         }
    //     }
    // }
    return pixels;
}
