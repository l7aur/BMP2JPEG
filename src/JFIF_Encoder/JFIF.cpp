#include "JFIF.h"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <map>

typedef uint16_t marker;
static constexpr unsigned int MARKER_SIZE_BYTES{ 2 };

static const std::map<const std::string, const marker> MARKERS {
            {"TEM", 0x01'FF},
            {"RST0", 0xD0'FF},{"RST1", 0xD1'FF},{"RST2", 0xD2'FF},{"RST3", 0xD3'FF},
            {"RST4", 0xD4'FF},{"RST5", 0xD5'FF},{"RST6", 0xD6'FF},{"RST7", 0xD7'FF},
            {"SOI", 0xD8'FF},
            {"EOI", 0xD9'FF},
            {"SOF0", 0xC0'FF}, {"SOF1", 0xC1'FF}, {"SOF2", 0xC2'FF},{"SOF3", 0xC3'FF},
            {"DHT", 0xC4'FF},
            {"SOS", 0xDA'FF},
            {"DQT", 0xDB'FF},
            {"DRI", 0xDD'FF},
            {"APP0", 0xE0'FF}, {"APP1", 0xE1'FF},{"APP2", 0xE2'FF},{"APP3", 0xE3'FF},
            {"APP4", 0xE4'FF}, {"APP5", 0xE5'FF},{"APP6", 0xE6'FF},{"APP7", 0xE7'FF},
            {"APP8", 0xE8'FF}, {"APP9", 0xE9'FF},{"APP10", 0xEA'FF},{"APP11", 0xEB'FF},
            {"APP12", 0xEC'FF}, {"APP13", 0xED'FF},{"APP14", 0xEE'FF},{"APP15", 0xEF'FF},
            {"COM", 0xFE'FF}
    /* more markers, but unused */
};

JFIF::JFIF(const std::string_view& folderPath, const std::string_view& fileName) {
    std::string filePathStr = folderPath.data();
    filePathStr.append("/");
    filePathStr.append(fileName);
    filePathStr.append(".jfif");
    fileDescriptor = creat(filePathStr.c_str(),  O_CREAT | S_IRUSR | S_IWUSR);
}

int JFIF::encode(const uint32_t *pixels, const int width, const int height) const {
    if (fileDescriptor < 0) {
        std::cerr << "[ERROR] .jfif file could not be created!\n";
        return -1;
    }
    if (lseek(fileDescriptor, SEEK_SET, 0) < 0) {
        std::cerr << "[ERROR] Could not place the descriptor at the beginning of the file!\n";
        return -1;
    }

    int status = writeMarker("SOI");
    if (status < 0) return -1;

    status = writeAPP0Header();
    if (status < 0) return -1;

    status = writeDQTMarker();
    if (status < 0) return -1;

    status = writeMarker("DHT");
    if (status < 0) return -1;

    status = writeMarker("SOS");
    if (status < 0) return -1;

    status = writeMarker("EOI");
    if (status < 0) return -1;

    std::cout << "[ERROR] Successfully created the .jfif file!\n";
    return 0;
}

void JFIF::print() const {
}

int JFIF::writeMarker(const char* markerId) const {
    try {
        const marker m = MARKERS.at(markerId);
        return static_cast<int>(write(fileDescriptor, &m, MARKER_SIZE_BYTES));
    }
    catch (const std::out_of_range&) {
        std::cerr << "[ERROR] Could not write marker " << markerId << " in .jfif file!\n";
        return -1;
    }
}

int JFIF::writeAPP0Header() const {
    if (writeMarker("APP0") < 0)
        return -1;

    std::string segmentData;
    segmentData.append("JFIF\0", 5);    // Identifier
    segmentData.append("\x01\x02", 2);  // Version major & minor
    segmentData.append("\x00", 1);      // Units
    segmentData.append("\x00\x00", 2);  // X density
    segmentData.append("\x00\x00", 2);  // Y density
    segmentData.append("\x00", 1);      // X thumbnail
    segmentData.append("\x00", 1);      // Y thumbnail
                                            // no Thumbnail
    return writeSegmentData(segmentData);
}

int JFIF::writeDQTMarker() const {
    if (writeMarker("DQT") < 0)
        return -1;

    std::string segmentData;
    // segmentData.append("", );
    return writeSegmentData(segmentData);
}

int JFIF::writeSegmentData(const std::string_view& segmentData) const {
    const uint16_t segmentSize = segmentData.size() + 2;
    const uint8_t sizeBuffer[2] = { static_cast<uint8_t>((segmentSize >> 8) & 0xFF), static_cast<uint8_t>(segmentSize & 0xFF) };

    if (write(fileDescriptor, sizeBuffer, sizeof(sizeBuffer)) < 0)
        return -1;
    if (write(fileDescriptor, segmentData.data(), segmentData.size()) < 0)
        return -1;

    return 0;
}

void JFIF::cleanup() const {
    if (close(fileDescriptor) < 0)
        std::cerr << "[ERROR] Could not close the .jfif file!\n";
}
