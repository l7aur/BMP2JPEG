#pragma once

#include <cstdint>

struct FileHeader;
struct DIBHeader;
struct PixelData;

/**
 * Object implementing a BMP image.
 * No validation of field values implemented.
 */
class BMP final
{
public:
    explicit BMP(const char *filePath);
    ~BMP() { cleanup(); }
    void print() const;
    int process();
    [[nodiscard]] const uint32_t *getPixelData() const ;
    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
    [[nodiscard]] int getFileDescriptor() const;
    [[nodiscard]] const char* getPath() const;

private:
    FileHeader *fileHeader{nullptr};
    DIBHeader *dibHeader{nullptr};
    PixelData *pixelData{nullptr};
    int fileDescriptor{-1};
    const char* path{nullptr};

    int processFileHeader();
    int processDIBHeader();
    int processPixelData();

    void cleanup() const;
};