#include "JFIF.h"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <map>
#include <array>

typedef uint16_t marker;
namespace {
    constexpr unsigned int MARKER_SIZE_BYTES{ 2 };

    const std::map<const std::string, const marker> MARKERS {
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
        /* more markers, but redundant */
    };

    /* taken from Digital Video Processing for Engineers, Suhel Dhanani, Michael Parker */
    constexpr std::array LUMINANCE_QUANTIZATION_TABLE {
        16, 11, 10, 16, 24, 40, 51, 61,
        12, 12, 14, 19, 26, 58, 60, 55,
        14, 13, 16, 24, 40, 57, 69, 56,
        14, 17, 22, 29, 51, 87, 80, 62,
        18, 22, 37, 56, 68, 109, 103, 77,
        24, 35, 55, 64, 81, 104, 113, 92,
        49, 64, 78, 87, 103, 121, 120, 101,
        72, 92, 95, 98, 112, 100, 103, 99
    };

    /* taken from Digital Video Processing for Engineers, Suhel Dhanani, Michael Parker */
    constexpr std::array CHROMINANCE_QUANTIZATION_TABLE {
        17, 18, 24, 47, 99, 99, 99, 99,
        18, 21, 26, 66, 99, 99, 99, 99,
        24, 26, 56, 99, 99, 99, 99, 99,
        47, 66, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99
    };

}

JFIF::JFIF(const std::string_view& folderPath, const std::string_view& fileName) {
    std::string filePathStr = folderPath.data();
    filePathStr.append("/");
    filePathStr.append(fileName);
    filePathStr.append(".jfif");
    fileDescriptor = creat(filePathStr.c_str(),  O_CREAT | S_IRUSR | S_IWUSR);
}

void JFIF::print() const {
}

int JFIF::encode(const uint32_t *pixels, const int width, const int height, int comprQuality) const {
    if (fileDescriptor < 0) {
        std::cerr << "[ERROR] .jfif file could not be created!\n";
        return -1;
    }
    if (lseek(fileDescriptor, SEEK_SET, 0) < 0) {
        std::cerr << "[ERROR] Could not place the descriptor at the beginning of the file!\n";
        return -1;
    }

    if (writeMarker("SOI") < 0) return -1;
    if (writeAPP0Header() < 0) return -1;
    // if (writeCOMMarker() < 0) return -1;
    if (writeDQTMarkers() < 0) return -1;
    if (writeMarker("DHT") < 0) return -1;
    if (writeMarker("SOS") < 0) return -1;
    if (writeMarker("EOI") < 0) return -1;

    std::cout << "[ERROR] Successfully created the .jfif file!\n";
    return 0;
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

    std::vector<uint8_t> segmentData;
    segmentData.insert(segmentData.end(), {'J', 'F', 'I', 'F', '\0'}); // Identifier
    segmentData.insert(segmentData.end(), {0x01, 0x02});               // Version major & minor
    segmentData.push_back(0x00);                                                // Units
    segmentData.insert(segmentData.end(), {0x00, 0x00});               // X density
    segmentData.insert(segmentData.end(), {0x00, 0x00});               // Y density
    segmentData.push_back(0x00);                                                // X thumbnail
    segmentData.push_back(0x00);                                                // Y thumbnail
                                                                                  // no Thumbnail
    return writeSegmentData(segmentData);
}

int JFIF::writeDQTMarkers() const {
    if (writeLuminanceDQTMarker() < 0) return -1;
    if (writeChrominanceDQTMarker() < 0) return -1;
    return 0;
}

int JFIF::writeCOMMarker() const {
    if (writeMarker("COM") < 0)
        return -1;

    return writeSegmentData(std::vector<uint8_t>{
        'T', 'H', 'I', 'S', ' ', 'I', 'S', ' ', 'A', ' ',
        'J', 'P', 'E', 'G', ' ', 'F', 'I', 'L', 'E', ' ',
        'G', 'E', 'N', 'E', 'R', 'A', 'T', 'E', 'D', ' ',
        'B', 'Y', ' ', 'L', '7', 'A', 'U', 'R', '\0'
    });
}

int JFIF::writeLuminanceDQTMarker() const {
    if (writeMarker("DQT") < 0)
        return -1;

    std::vector<uint8_t> segmentData;
    segmentData.push_back(0b0000'0000);  // table identifier ' quantization values: 1 byte unsigned
    for (const auto& i: LUMINANCE_QUANTIZATION_TABLE)
        segmentData.push_back(static_cast<uint8_t>(i));
    return writeSegmentData(segmentData);
}

int JFIF::writeChrominanceDQTMarker() const {
    if (writeMarker("DQT") < 0)
        return -1;

    std::vector<uint8_t> segmentData;
    segmentData.push_back(0b0001'0000);  // table identifier ' quantization values: 1 byte unsigned
    for (const auto& i: CHROMINANCE_QUANTIZATION_TABLE)
        segmentData.push_back(static_cast<uint8_t>(i));
    return writeSegmentData(segmentData);
}

int JFIF::writeSegmentData(const std::vector<uint8_t>& segmentData) const {
    static constexpr int MARKER_LENGTH_SIZE = 2;
    const uint16_t segmentSize = segmentData.size() + MARKER_LENGTH_SIZE;
    const uint8_t sizeBuffer[MARKER_LENGTH_SIZE] = { static_cast<uint8_t>((segmentSize >> 8) & 0xFF), static_cast<uint8_t>(segmentSize & 0xFF) };

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
