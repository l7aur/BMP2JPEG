#include "ColorTable.h"

#include <unistd.h>

int ColorTable::initFrom(const int fd) const
{
    if (read(fd, data, tableSize) < 0)
    {
        std::cerr << "Reading the color table failed!\n";
        return -1;
    }
    return 0;
}

Pixel ColorTable::at(const size_t i) const {
    if (i > tableSize)
        return Pixel{0xFF, 0xFF, 0xFF, 0xFF};
    return Pixel{data[i], data[i + 1], data[i + 2], (data[i + 3] != 0x00) ? data[i + 3] : static_cast<uint8_t>(0xFF)};
}

void ColorTable::print() const
{
    std::cout << "===============COLOR--TAB================\n";
    for (size_t i = 0; i < tableSize; i += 4)
        printf("%zi\t-> R: %02X  G: %02X  B: %02X  U:  %02X\n", i / 4, data[i], data[i + 1], data[i + 2], data[i + 3]);
    std::cout << "===============COLOR--TAB================\n";
}
