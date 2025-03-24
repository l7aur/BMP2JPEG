#include "BMP.h"

BMP::BMP(const char *filePath)
    : Image(filePath)
{
}

void BMP::print() const
{
    if (fileHeader)
        fileHeader->print();
    if (dibHeader)
        dibHeader->print();
    if (pixelData)
        pixelData->print();
}

int BMP::process()
{
    if (processFileHeader() < 0 || processDIBHeader() < 0 || /*processEBMask() < 0 ||*/ processPixelData())
        return -1;
    return 0;
}

const uint32_t * BMP::getPixelData() const {
    return pixelData->getAndFormatData(
        static_cast<int>(dibHeader->getWidth()),
        static_cast<int>(dibHeader->getHeight()));
}

int BMP::processFileHeader()
{
    fileHeader = new FileHeader();
    return fileHeader->initFrom(fileDescriptor);
}

int BMP::processDIBHeader()
{
    uint32_t dibSize{0};
    if (read(fileDescriptor, &dibSize, sizeof(dibSize)) < 0)
    {
        std::cerr << "Error while reading the size of the DIB header!\n";
        return -1;
    }
    dibHeader = new DIBHeader(dibSize);
    return dibHeader->initFrom(fileDescriptor, dibSize);
}

// int BMP::processEBMask()
// {
//     return 0;
// }

int BMP::processPixelData()
{
    pixelData = new PixelData(MAXIMUM_FILE_SIZE_IN_BYTES);
    if (pixelData->initFrom(fileDescriptor, fileHeader->pixelDataStartingAddress, dibHeader->getBitCount(), dibHeader->getCompression()) < 0)
    {
        std::cerr << "Pixel data initialization failed!\n";
        return -1;
    }
    return 0;
}

void BMP::cleanup() const {
    if (fileHeader)
        free(fileHeader);
    delete dibHeader;
    delete pixelData;
    // if (extraBitMask)
    // free(extraBitMask);

}
