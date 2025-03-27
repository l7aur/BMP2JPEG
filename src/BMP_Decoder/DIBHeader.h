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
    [[nodiscard]] uint32_t getSizeOfColorTable() const;
    [[nodiscard]] uint16_t getBitCount() const;
    [[nodiscard]] uint32_t getCompression() const;
    [[nodiscard]] int32_t getWidth() const;
    [[nodiscard]] int32_t getHeight() const;
    void print() const;

private:
    [[nodiscard]] int init_BITMAPINFOHEADER(int fd) const;
    void printHelper_BITMAPINFOHEADER() const;

    [[nodiscard]] uint16_t getBitCount_BITMAPINFOHEADER() const;
    [[nodiscard]] uint32_t getCompression_BITMAPINFOHEADER() const;
    [[nodiscard]] int32_t getWidth_BITMAPINFOHEADER() const;
    [[nodiscard]] int32_t getHeight_BITMAPINFOHEADER() const;
};