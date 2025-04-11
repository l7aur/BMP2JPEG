#include "FileHeader.h"

#include <iostream>
#include <unistd.h>

int FileHeader::initFrom(const int fd)
{
    if (read(fd, headerField, HF_BSIZE) < 0)
    {
        std::cerr << "[ERROR] Could not read \'fileHeader->headerField\'!\n";
        return -1;
    }
    if (read(fd, &fileSize, FS_BSIZE) < 0)
    {
        std::cerr << "[ERROR] Could not read \'fileHeader->fileSize\'!\n";
        return -1;
    }
    if (read(fd, reservedField1, RF1_BSIZE) < 0)
    {
        std::cerr << "[ERROR] Could not read \'fileHeader->reservedField1\'!\n";
        return -1;
    }
    if (read(fd, reservedField2, RF2_BSIZE) < 0)
    {
        std::cerr << "[ERROR] Could not read \'fileHeader->reservedField2\'!\n";
        return -1;
    }
    if (read(fd, &pixelDataStartingAddress, PDSA_BSIZE) < 0)
    {
        std::cerr << "[ERROR] Could not read \'fileHeader->reservedField2\'!\n";
        return -1;
    }
    return 0;
}

void FileHeader::print() const
{
    printf("===============BMP--HEADER===============\n");
    printf("Header field: %c%c\n", headerField[0], headerField[1]);
    printf("File size: %08X\n", fileSize);
    printf("Reserved field 1: %02X %02X\n", reservedField1[0], reservedField1[1]);
    printf("Reserved field 2: %02X %02X\n", reservedField2[0], reservedField2[1]);
    printf("Pixel data starting address: %08X\n", pixelDataStartingAddress);
    printf("==============END-OF-HEADER==============\n");
}
