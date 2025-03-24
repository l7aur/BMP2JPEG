#pragma once

#include "../../Core/Image.h"
#include "FileHeader.h"
#include "DIBHeader.h"
#include "PixelData.h"

/**
 * Object implementing a BMP image.
 * No validation of field values implemented.
 */
class BMP final : public Image
{
public:
    explicit BMP(const char *filePath);
    ~BMP() override { cleanup(); }
    void print() const override;
    int process() override;
    [[nodiscard]] const uint32_t *getPixelData() const ;
    [[nodiscard]] int getWidth() const { return static_cast<int>(dibHeader->getWidth()); };
    [[nodiscard]] int getHeight() const { return static_cast<int>(dibHeader->getHeight()); };

private:
    FileHeader *fileHeader{nullptr};
    DIBHeader *dibHeader{nullptr};
    // ExtraBitMask *extraBitMask{ nullptr };
    PixelData *pixelData{nullptr};

    /* process helpers */
    int processFileHeader();
    int processDIBHeader();
    // int processEBMask();
    int processPixelData();

    /* destructor helper */
    void cleanup() const;
};