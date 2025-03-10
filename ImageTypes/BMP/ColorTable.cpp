#include "ColorTable.h"

int ColorTable::initFrom(const int fd)
{
    if (read(fd, data, tableSize) < 0)
    {
        std::cerr << "Reading the color table failed!\n";
        return -1;
    }
    return 0;
}

Pixel ColorTable::at(size_t i)
{
    if (i > tableSize)
        return Pixel{};
    return Pixel{data[i], data[i + 1], data[i + 2], (data[i + 3] != 0x00) ? data[i + 3] : static_cast<uint8_t>(0xFF)};
}

void ColorTable::print()
{
    std::cout << "===============COLOR--TAB================\n";
    for (size_t i = 0; i < tableSize; i += 4)
        printf("%zi\t-> R: %02X  G: %02X  B: %02X  U:  %02X\n", i / 4, data[i], data[i + 1], data[i + 2], data[i + 3]);
    std::cout << "===============COLOR--TAB================\n";
}
