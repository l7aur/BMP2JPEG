#include "DIBHeader.h"

#include <iostream>
#include <unistd.h>

DIBHeader::DIBHeader(const uint32_t in_type)
{
    switch (in_type)
    {
    case BITMAPCOREHEADER:
        type = BITMAPCOREHEADER;
        data = malloc(sizeof(BITMAPCOREHEADER_T));
        break;
    case OS22XBITMAPHEADER:
        type = OS22XBITMAPHEADER;
        data = malloc(sizeof(OS22XBITMAPHEADER_T));
        break;
    case BITMAPINFOHEADER:
        type = BITMAPINFOHEADER;
        data = malloc(sizeof(BITMAPINFOHEADER_T));
        break;
    case BITMAPV2INFOHEADER:
        type = BITMAPV2INFOHEADER;
        data = malloc(sizeof(BITMAPV2INFOHEADER_T));
        break;
    case BITMAPV3INFOHEADER:
        type = BITMAPV3INFOHEADER;
        data = malloc(sizeof(BITMAPV3INFOHEADER_T));
        break;
    case BITMAPV4HEADER:
        type = BITMAPV4HEADER;
        data = malloc(sizeof(BITMAPV4HEADER_T));
        break;
    case BITMAPV5HEADER:
        type = BITMAPV5HEADER;
        data = malloc(sizeof(BITMAPV5HEADER_T));
        break;
    default:
        std::cerr << "Unhandled DIB header structure!\n";
        break;
    }
}

DIBHeader::~DIBHeader()
{
    if (data)
        free(data);
}

int DIBHeader::initFrom(const int fd, const uint32_t dibSize) const {
    switch (type) {
        case BITMAPINFOHEADER:
            static_cast<BITMAPINFOHEADER_T *>(data)->size = dibSize;
        return init_BITMAPINFOHEADER(fd);
        default:
            std::cerr << "DIB header structure is not handled!\n";
        return -1;
    }
}

void DIBHeader::print() const
{
    switch (type) {
        case BITMAPINFOHEADER:
            printHelper_BITMAPINFOHEADER();
        break;
        default:
            std::cerr << "DIB header structure is not handled!\n";
        break;
    }
}

uint32_t DIBHeader::getSizeOfColorTable() const
{
    if (data == nullptr)
        return 0;
    switch (type)
    {
    case BITMAPINFOHEADER:
    {
        auto *ptr = static_cast<BITMAPINFOHEADER_T *>(data);
        if (ptr->compression == BI_RGB && ptr->bitCount < 8)
            return ptr->usedColors;
        return 0;
    }
    default:
        std::cerr << "DIB header structure is not handled!\n";
        break;
    }
    return 0;
}

uint16_t DIBHeader::getBitCount() const
{
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getBitCount_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'bitCount\' for dib header of size " << type << "\n";
        return 0;
    }
}

uint32_t DIBHeader::getCompression() const
{
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getCompression_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'compression\' for dib header of size " << type << "\n";
        return 0;
    }
}

int32_t DIBHeader::getWidth() const
{
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getWidth_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'compression\' for dib header of size " << type << "\n";
        return 0;
    }
}

int32_t DIBHeader::getHeight() const
{
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getHeight_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'compression\' for dib header of size " << type << "\n";
        return 0;
    }
}

int DIBHeader::init_BITMAPINFOHEADER(const int fd) const
{
    auto *ptr = static_cast<BITMAPINFOHEADER_T *>(data);
    if (read(fd, &(ptr->width), sizeof(ptr->width)) < 0)
    {
        std::cerr << "Error while reading the \'width\'\n";
        return -1;
    }
    if (read(fd, &(ptr->height), sizeof(ptr->height)) < 0)
    {
        std::cerr << "Error while reading the \'height\'\n";
        return -1;
    }
    if (read(fd, &(ptr->planes), sizeof(ptr->planes)) < 0)
    {
        std::cerr << "Error while reading the \'planes\'\n";
        return -1;
    }
    if (read(fd, &(ptr->bitCount), sizeof(ptr->bitCount)) < 0)
    {
        std::cerr << "Error while reading the \'bitCount\'\n";
        return -1;
    }
    if (read(fd, &(ptr->compression), sizeof(ptr->compression)) < 0)
    {
        std::cerr << "Error while reading the \'compression\'\n";
        return -1;
    }
    if (read(fd, &(ptr->imageSize), sizeof(ptr->imageSize)) < 0)
    {
        std::cerr << "Error while reading the \'imageSize\'\n";
        return -1;
    }
    if (read(fd, &(ptr->pixelsPerMeterOX), sizeof(ptr->pixelsPerMeterOX)) < 0)
    {
        std::cerr << "Error while reading the \'pixelsPerMeterOX\'\n";
        return -1;
    }
    if (read(fd, &(ptr->pixelsPerMeterOY), sizeof(ptr->pixelsPerMeterOY)) < 0)
    {
        std::cerr << "Error while reading the \'pixelsPerMeterOY\'\n";
        return -1;
    }
    if (read(fd, &(ptr->usedColors), sizeof(ptr->usedColors)) < 0)
    {
        std::cerr << "Error while reading the \'usedColors\'\n";
        return -1;
    }
    if (read(fd, &(ptr->importantColors), sizeof(ptr->importantColors)) < 0)
    {
        std::cerr << "Error while reading the \'importantColors\'\n";
        return -1;
    }
    return 0;
}

void DIBHeader::printHelper_BITMAPINFOHEADER() const
{
    const auto *ptr = static_cast<BITMAPINFOHEADER_T *>(data);
    printf("===============DIB--HEADER===============\n");
    printf("Size:\t\t\t\t%08X\n", ptr->size);
    printf("Width:\t\t\t\t%08X\n", ptr->width);
    printf("Height:\t\t\t\t%08X\n", ptr->height);
    printf("Planes:\t\t\t\t%04X\n", ptr->planes);
    printf("Bit Count:\t\t\t%04X\n", ptr->bitCount);
    printf("Compression:\t\t\t%08X\n", ptr->compression);
    printf("Image size:\t\t\t%08X\n", ptr->imageSize);
    printf("Pixels per meter OX:\t\t%08X\n", ptr->pixelsPerMeterOX);
    printf("Pixels per meter OY:\t\t%08X\n", ptr->pixelsPerMeterOY);
    printf("Used colors:\t\t\t%08X\n", ptr->usedColors);
    printf("Important colors:\t\t%08X\n", ptr->importantColors);
    printf("============END-OF-DIB-HEADER============\n");
}

uint16_t DIBHeader::getBitCount_BITMAPINFOHEADER() const
{
    return static_cast<BITMAPINFOHEADER_T *>(data)->bitCount;
}

uint32_t DIBHeader::getCompression_BITMAPINFOHEADER() const
{
    return static_cast<BITMAPINFOHEADER_T *>(data)->compression;
}

int32_t DIBHeader::getWidth_BITMAPINFOHEADER() const
{
    return std::max(0, static_cast<BITMAPINFOHEADER_T *>(data)->width);
}

int32_t DIBHeader::getHeight_BITMAPINFOHEADER() const
{
    return std::max(0, static_cast<BITMAPINFOHEADER_T *>(data)->height);
}
