#include "ColorTable.h"

#include <unistd.h>

int ColorTable::initFrom(const int fd) const {
    if (read(fd, data, tableSize) < 0)
    {
        std::cerr << "Reading the color table failed!\n";
        return -1;
    }
    return 0;
}

Pixel ColorTable::at(const uint8_t i) const {
    const int index = 4 * i;
    if (index > tableSize)
        return Pixel{0xFF, 0xFF, 0xFF, 0xFF};
    return Pixel{data[index + 2], data[index + 1], data[index], data[index + 3]};
}

void ColorTable::print() const {
    std::cout << "===============COLOR--TAB================\n";
    for (size_t i = 0; i < tableSize; i += 4)
        printf("%zi\t-> R: %02X  G: %02X  B: %02X  U:  %02X\n", i / 4, data[i], data[i + 1], data[i + 2], data[i + 3]);
    std::cout << "===============COLOR--TAB================\n";
}
