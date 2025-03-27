#include "DIBHeader.h"

#include <iostream>
#include <unistd.h>

struct BITMAPCOREHEADER_T {

};
struct OS22XBITMAPHEADER_T {

};
struct BITMAPINFOHEADER_T {
    uint32_t size{0};                       // size of the structure in bytes
    int32_t width{0};                       // width in pixels
    int32_t height{0};                      // height in pixels
                                            // for uncompressed RGB bitmaps, if it positive the bitmap is bottom-up, with the DIB origin at the lower left corner
                                            //                               if it is negative the bitmap is a top-down DIB with the origin at the upper right corner
                                            // for YUV bitmaps, the bitmap is always top-down regardless of sign
                                            // for compressed formats it has to be positive
    uint16_t planes{0};                     // number of planes, must be 1
    uint16_t bitCount{0};                   // number of bits per pixel, for uncompressed formats it is the average of bits per pixel
                                            //                           for compressed formats it is the bit depth of the uncompressed image after its decompression
    uint32_t compression{0};                //
    uint32_t imageSize{0};                  // size of the image in bytes
    int32_t pixelsPerMeterOX{0};            // horizontal resolution, in pixels per meter, of the target device for the bitmap
    int32_t pixelsPerMeterOY{0};            // vertical resolution, in pixels per meter, of the target device for the bitmap
    uint32_t usedColors{0};                 // number of color indices in the color table that are actually used by the bitmap
    uint32_t importantColors{0};            // number of color indices that are considered important for displaying the bitmap, if it is 0 all colors are important
};
struct BITMAPV2INFOHEADER_T {
};
struct BITMAPV3INFOHEADER_T {
};
struct BITMAPV4HEADER_T {
};
struct BITMAPV5HEADER_T {
    // test the types
    // uint32_t size{0x0000'0000};
    // uint64_t width{0x0000'0000'0000'0000};
    // uint64_t height{0x0000'0000'0000'0000};
    // uint16_t planes{0x0000};
    // uint16_t bitCount{0x0000};
    // uint32_t compression{0x0000'0000};
    // uint32_t imageSize{0x0000'0000};
    // uint64_t pixelsPerMeterOX{0x0000'0000'0000'0000};
    // uint64_t pixelsPerMeterOY{0x0000'0000'0000'0000};
    // uint32_t usedColors{0x0000'0000};      // number of color indices in the color table that are actually used by the bitmap
    // uint32_t importantColors{0x0000'0000}; // number of color indices that are considered important for displaying the bitmap
    // uint32_t redMask{0x0000'0000};
    // uint32_t greenMask{0x0000'0000};
    // uint32_t blueMask{0x0000'0000};
    // uint32_t alphaMask{0x0000'0000};
    // uint32_t csType{0x0000'0000};
    // struct
    // {
    //     uint32_t x1, y1, z1;
    //     uint32_t x2, y2, z2;
    //     uint32_t x3, y3, z3;
    // } endpoints{
    //     0x0000'0000, 0x0000'0000, 0x0000'0000,
    //     0x0000'0000, 0x0000'0000, 0x0000'0000,
    //     0x0000'0000, 0x0000'0000, 0x0000'0000};
    // uint32_t redGamma{0x0000'0000};
    // uint32_t greenGamma{0x0000'0000};
    // uint32_t blueGamma{0x0000'0000};
    // uint32_t intent{0x0000'0000};
    // uint32_t profileData{0x0000'0000};
    // uint32_t profileSize{0x0000'0000};
    // uint32_t reserved{0x0000'0000};
};

DIBHeader::DIBHeader(const uint32_t in_type) {
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

DIBHeader::~DIBHeader() {
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

uint32_t DIBHeader::getSizeOfColorTable() const {
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

uint16_t DIBHeader::getBitCount() const {
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getBitCount_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'bitCount\' for dib header of size " << type << "\n";
        return 0;
    }
}

uint32_t DIBHeader::getCompression() const {
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getCompression_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'compression\' for dib header of size " << type << "\n";
        return 0;
    }
}

int32_t DIBHeader::getWidth() const {
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getWidth_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'compression\' for dib header of size " << type << "\n";
        return 0;
    }
}

int32_t DIBHeader::getHeight() const {
    switch (type)
    {
    case BITMAPINFOHEADER:
        return getHeight_BITMAPINFOHEADER();
    default:
        std::cerr << "Unable to retrieve \'compression\' for dib header of size " << type << "\n";
        return 0;
    }
}

int DIBHeader::init_BITMAPINFOHEADER(const int fd) const {
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

void DIBHeader::print() const {
    switch (type) {
        case BITMAPINFOHEADER:
            printHelper_BITMAPINFOHEADER();
        break;
        default:
            std::cerr << "DIB header structure is not handled!\n";
        break;
    }
}

void DIBHeader::printHelper_BITMAPINFOHEADER() const {
    const auto *ptr = static_cast<BITMAPINFOHEADER_T *>(data);
    printf("===============DIB--HEADER===============\n");
    printf("Size: %08X\n", ptr->size);
    printf("Width: %08X\n", ptr->width);
    printf("Height: %08X\n", ptr->height);
    printf("Planes: %04X\n", ptr->planes);
    printf("Bit Count: %04X\n", ptr->bitCount);
    printf("Compression: %08X\n", ptr->compression);
    printf("Image size: %08X\n", ptr->imageSize);
    printf("Pixels per meter OX: %08X\n", ptr->pixelsPerMeterOX);
    printf("Pixels per meter OY: %08X\n", ptr->pixelsPerMeterOY);
    printf("Used colors: %08X\n", ptr->usedColors);
    printf("Important colors: %08X\n", ptr->importantColors);
    printf("============END-OF-DIB-HEADER============\n");
}

uint16_t DIBHeader::getBitCount_BITMAPINFOHEADER() const {
    return static_cast<BITMAPINFOHEADER_T *>(data)->bitCount;
}

uint32_t DIBHeader::getCompression_BITMAPINFOHEADER() const {
    return static_cast<BITMAPINFOHEADER_T *>(data)->compression;
}

int32_t DIBHeader::getWidth_BITMAPINFOHEADER() const {
    return std::max(0, static_cast<BITMAPINFOHEADER_T *>(data)->width);
}

int32_t DIBHeader::getHeight_BITMAPINFOHEADER() const {
    return std::max(0, static_cast<BITMAPINFOHEADER_T *>(data)->height);
}
