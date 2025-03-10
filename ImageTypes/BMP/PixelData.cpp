#include "PixelData.h"

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

const uint32_t *PixelData::getAndFormatData(const int imageWidth, const int imageHeight, const int byteWidth) const
{
    switch (encoding)
    {
    case C8:
        return format_C8(imageWidth, imageHeight, byteWidth);
    default:
        break;
    }
    std::cerr << "Unknown format! Unable to format pixel data!\n";
    return nullptr;
}

const uint32_t *PixelData::format_C8(const int imageWidth, const int imageHeight, const int byteWidth) const
{
    auto *formatted = new uint32_t[dataSize];
    std::cout << imageWidth << ' ' << imageHeight << '\n';
    // unsigned int lineCounter = 0;
    for (size_t i = 0; i < dataSize; i++)
    {
        // if (byteWidth * 8 %  != 0 && i * 4 >= lineCounter * byteWidth + imageWidth) // padding
        // {
        //     std::cout << "last pixel index: " << i << ' ' << byteWidth << '\n';
        //     int c = 0;
        //     lineCounter++;
        //     while (i < formattedSize && i <= lineCounter * static_cast<unsigned int>(byteWidth))
        //         c++, formatted[i++] = 0xFF'FF'FF'FF;
        //     // i--;
        //     std::cout << "counter padding " << c << '\n';
        //     continue;
        // }
        Pixel p = colorTab->at(data[i]);
        std::cout << static_cast<int>(data[i]) << ' ' << p << '\n';
        formatted[i] = (p.r << 24) | (p.g << 16) | (p.b << 8) | p.a;
    }
    // for (size_t i = 0; i < 438; i++)
    // formatted[i] = 0xFF'00'00'FF;
    return formatted;
}
