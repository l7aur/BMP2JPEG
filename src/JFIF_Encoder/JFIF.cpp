#include "JFIF.h"

#include <fcntl.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <map>

typedef uint16_t marker;
static constexpr unsigned int MARKER_SIZE_BYTES{2};

static const std::map<std::string, marker> MARKERS {
    {"TEM", 0xFF'01},
    {"RST0", 0xFF'D0},{"RST1", 0xFF'D1},{"RST2", 0xFF'D2},{"RST3", 0xFF'D3},
    {"RST4", 0xFF'D4},{"RST5", 0xFF'D5},{"RST6", 0xFF'D6},{"RST7", 0xFF'D7},
    {"SOI", 0xFF'D8},
    {"EOI", 0xFF'D9},
    {"SOF0", 0xFF'C0}, {"SOF1", 0xFF'C1}, {"SOF2", 0xFF'C2},{"SOF3", 0xFF'C3},
    {"DHT", 0xFF'C4},
    {"SOS", 0xFF'DA},
    {"DQT", 0xFF'DB},
    {"DRI", 0xFF'DD},
    {"APP0", 0xFF'E0}, {"APP1", 0xFF'E1},{"APP2", 0xFF'E2},{"APP3", 0xFF'E3},
    {"APP4", 0xFF'E4}, {"APP5", 0xFF'E5},{"APP6", 0xFF'E6},{"APP7", 0xFF'E7},
    {"APP8", 0xFF'E8}, {"APP9", 0xFF'E9},{"APP10", 0xFF'EA},{"APP11", 0xFF'EB},
    {"APP12", 0xFF'EC}, {"APP13", 0xFF'ED},{"APP14", 0xFF'EE},{"APP15", 0xFF'EF},
    {"COM", 0xFF'FE}
    /* not all markers */
};

JFIF::JFIF(const char *folderPath, const char *fileName) {
    std::string filePathStr = folderPath;
    filePathStr.append("/");
    filePathStr.append(fileName);
    filePathStr.append(".jfif");
    fileDescriptor = creat(filePathStr.c_str(),  O_CREAT | S_IRUSR | S_IWUSR);
}

int JFIF::encode(const uint32_t *pixels, const int width, const int height) const {
    if (fileDescriptor < 0) {
        std::cerr << ".jfif file could not be created!\n";
        return -1;
    }

    int status = writeMarker("SOI");
    if (status < 0) return -1;

    status = writeMarker("APP0");
    if (status < 0) return -1;

    status = writeMarker("COM");
    if (status < 0) return -1;

    status = writeMarker("SOF0");
    if (status < 0) return -1;

    status = writeMarker("DQT");
    if (status < 0) return -1;

    status = writeMarker("DRI");
    if (status < 0) return -1;

    status = writeMarker("DHT");
    if (status < 0) return -1;

    status = writeMarker("SOS");
    if (status < 0) return -1;

    status = writeMarker("EOI");
    if (status < 0) return -1;

    return 0;
}

int JFIF::writeMarker(const char* markerId) const {
    try {
        const marker m = MARKERS.at(markerId);
        return static_cast<int>(write(fileDescriptor, &m, MARKER_SIZE_BYTES));
    }
    catch (const std::out_of_range&) {
        std::cerr << "Could not write marker " << markerId << " in .jfif file!\n";
        return -1;
    }
}

void JFIF::cleanup() const {
    if (close(fileDescriptor) < 0)
        std::cerr << "Error closing the .jfif file!\n";
}

