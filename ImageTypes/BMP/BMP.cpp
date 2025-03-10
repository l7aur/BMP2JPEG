#include "BMP.h"
#include "../../Core/Constants.h"

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
    setByteWidth();
    return 0;
}

void BMP::setByteWidth()
{
    const unsigned int bitWidth = static_cast<unsigned int>(dibHeader->getWidth()) << 3;
    const unsigned int r = bitWidth % BIT_ALIGNMENT;
    if (r == 0)
        return;
    byteWidth += (BIT_ALIGNMENT - r);
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
    if (pixelData->initFrom(fileDescriptor, dibHeader->getBitCount(), dibHeader->getCompression()) < 0)
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
