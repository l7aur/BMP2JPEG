#pragma once

#include <cstdint>
#include <string>

enum DIB_TYPES
{
    BITMAPCOREHEADER = 12, // Same as OS21XBITMAPHEADER
    OS22XBITMAPHEADER = 64,
    BITMAPINFOHEADER = 40,
    BITMAPV2INFOHEADER = 52,
    BITMAPV3INFOHEADER = 56,
    BITMAPV4HEADER = 108,
    BITMAPV5HEADER = 124
};

enum COMPRESSION_TYPE
{
    BI_RGB = 0x0000,
    BI_RLE8 = 0x0001,
    BI_RLE4 = 0x0002,
    BI_BITFIELDS = 0x0003,
    BI_JPEG = 0x0004,
    BI_PNG = 0x0005,
    BI_CMYK = 0x000B,
    BI_CMYKRLE8 = 0x000C,
    BI_CMYKRLE4 = 0x000D
};

struct BITMAPCOREHEADER_T
{

};
struct OS22XBITMAPHEADER_T
{

};
struct BITMAPINFOHEADER_T
{
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
struct BITMAPV2INFOHEADER_T
{
};
struct BITMAPV3INFOHEADER_T
{
};
struct BITMAPV4HEADER_T
{
};
struct BITMAPV5HEADER_T
{
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

/**
 * Object storing a BMP DIB header.
 */
class DIBHeader
{
public:
    DIB_TYPES type{BITMAPCOREHEADER};
    void *data{nullptr};
    explicit DIBHeader(uint32_t in_type);
    ~DIBHeader();
    [[nodiscard]] int initFrom(int fd, uint32_t dibSize) const;
    void print() const;
    [[nodiscard]] uint32_t getSizeOfColorTable() const;
    [[nodiscard]] uint16_t getBitCount() const;
    [[nodiscard]] uint32_t getCompression() const;
    [[nodiscard]] int32_t getWidth() const;
    [[nodiscard]] int32_t getHeight() const;
    
private:
    [[nodiscard]] int init_BITMAPINFOHEADER(int fd) const;
    void printHelper_BITMAPINFOHEADER() const;

    [[nodiscard]] uint16_t getBitCount_BITMAPINFOHEADER() const;
    [[nodiscard]] uint32_t getCompression_BITMAPINFOHEADER() const;
    [[nodiscard]] int32_t getWidth_BITMAPINFOHEADER() const;
    [[nodiscard]] int32_t getHeight_BITMAPINFOHEADER() const;
};