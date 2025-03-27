#include "BMP.h"

#include <fcntl.h>
#include <unistd.h>

#include "FileHeader.h"
#include "DIBHeader.h"
#include "PixelData.h"

static constexpr unsigned int MAXIMUM_FILE_SIZE_IN_BYTES {500'000};

BMP::BMP(const char *filePath)
    : path{filePath}
{
    fileDescriptor = open(filePath, O_RDONLY);
}

void BMP::print() const {
    if (fileHeader)
        fileHeader->print();
    if (dibHeader)
        dibHeader->print();
    if (pixelData)
        pixelData->print();
}

int BMP::process() {
    if (processFileHeader() < 0 || processDIBHeader() < 0 || /*processEBMask() < 0 ||*/ processPixelData())
        return -1;
    return 0;
}

const uint32_t * BMP::getPixelData() const {
    return pixelData->getAndFormatData(
        dibHeader->getWidth(),
        dibHeader->getHeight());
}

int BMP::getWidth() const {
    return dibHeader->getWidth();
}

int BMP::getHeight() const {
    return dibHeader->getHeight();
}

int BMP::getFileDescriptor() const {
    return fileDescriptor;
}

const char * BMP::getPath() const {
    return path;
}

int BMP::processFileHeader() {
    fileHeader = new FileHeader();
    return fileHeader->initFrom(fileDescriptor);
}

int BMP::processDIBHeader() {
    uint32_t dibSize{0};
    if (read(fileDescriptor, &dibSize, sizeof(dibSize)) < 0)
    {
        std::cerr << "Error while reading the size of the DIB header!\n";
        return -1;
    }
    dibHeader = new DIBHeader(dibSize);
    return dibHeader->initFrom(fileDescriptor, dibSize);
}

int BMP::processPixelData() {
    pixelData = new PixelData(MAXIMUM_FILE_SIZE_IN_BYTES);
    if (pixelData->initFrom(fileDescriptor, fileHeader->pixelDataStartingAddress, dibHeader->getBitCount(), dibHeader->getCompression()) < 0)
    {
        std::cerr << "Pixel data initialization failed!\n";
        return -1;
    }
    return 0;
}

void BMP::cleanup() const {
    if (close(fileDescriptor) < 0)
        std::cerr << "Error closing the .bmp file!\n";
    if (fileHeader)
        free(fileHeader);
    delete dibHeader;
    delete pixelData;

}
