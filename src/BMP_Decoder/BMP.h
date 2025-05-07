#pragma once

#include <cstdint>
#include <string>

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
    explicit BMP(std::string filePath);
    ~BMP() { cleanup(); }

    [[nodiscard]] const uint32_t *getPixelData() const ;
    [[nodiscard]] int getWidth() const;
    [[nodiscard]] int getHeight() const;
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] std::string getPath() const;

    int process();
    void print() const;

private:
    FileHeader *fileHeader{nullptr};
    DIBHeader *dibHeader{nullptr};
    PixelData *pixelData{nullptr};
    int fileDescriptor{-1};
    const std::string path{};

    int processFileHeader();
    int processDIBHeader();
    int processPixelData();

    void cleanup() const;
};