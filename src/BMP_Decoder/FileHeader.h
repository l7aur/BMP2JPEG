#pragma once

#include <cstdint>

/**
 * Object storing a BMP file header.
 */
struct FileHeader
{
    static constexpr unsigned int HF_BSIZE{2};
    static constexpr unsigned int FS_BSIZE{4};
    static constexpr unsigned int RF1_BSIZE{2};
    static constexpr unsigned int RF2_BSIZE{2};
    static constexpr unsigned int PDSA_BSIZE{4};

    uint8_t headerField[HF_BSIZE]{0x00, 0x00};
    uint32_t fileSize{0x0000'0000};
    uint8_t reservedField1[RF1_BSIZE]{0x00, 0x00};
    uint8_t reservedField2[RF2_BSIZE]{0x00, 0x00};
    uint32_t pixelDataStartingAddress{0x0000'0000};

    FileHeader() = default;
    ~FileHeader() = default;
    int initFrom(int fd);
    void print() const;
};