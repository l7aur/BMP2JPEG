#pragma once

#include <cstdint>
#include <string>

enum DIB_TYPES
{
    BITMAP_CORE_HEADER = 12, // Same as OS21X_BITMAP_HEADER
    OS22X_BITMAP_HEADER = 64,
    BITMAP_INFO_HEADER = 40,
    BITMAP_V2_INFO_HEADER = 52,
    BITMAP_V3_INFO_HEADER = 56,
    BITMAP_V4_HEADER = 108,
    BITMAP_V5_HEADER = 124
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
    DIB_TYPES type{BITMAP_INFO_HEADER};
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
    [[nodiscard]] int initBitmapInfoHeaderType(int fd) const;
    void printBitmapInfoHeader() const;

    [[nodiscard]] uint16_t getBitCountBitmapInfoHeader() const;
    [[nodiscard]] uint32_t getCompressionBitmapInfoHeader() const;
    [[nodiscard]] int32_t getWidthBitmapInfoHeader() const;
    [[nodiscard]] int32_t getHeightBitmapInfoHeader() const;
};