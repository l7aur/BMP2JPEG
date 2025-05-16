#include "JFIF.h"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <map>
#include <array>
#include <cassert>

#include "../Util/FreeFunctions.h"
#include "../Util/Mat8x8.h"

typedef uint16_t marker;

struct ProcessedPixelsType {
    std::vector<int> yValue;
    std::vector<int> cbValue;
    std::vector<int> crValue;
};

namespace {
    constexpr unsigned int MARKER_SIZE_BYTES{ 2 };
    constexpr unsigned int QUANTIZATION_COMPRESSION{ 2 };
    constexpr int BLOCK_SIZE{ 8 };

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
        0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08,
        0x07, 0x07, 0x07, 0x09, 0x09, 0x08, 0x0a, 0x0c, 0x14, 0x0d, 0x0c, 0x0b, 0x0b, 0x0c, 0x19, 0x12,
        0x13, 0x0f, 0x14, 0x1d, 0x1a, 0x1f, 0x1e, 0x1d, 0x1a, 0x1c, 0x1c, 0x20, 0x24, 0x2e, 0x27, 0x20,
        0x22, 0x2c, 0x23, 0x1c, 0x1c, 0x28, 0x37, 0x29, 0x2c, 0x30, 0x31, 0x34, 0x34, 0x34, 0x1f, 0x27,
        0x39, 0x3d, 0x38, 0x32, 0x3c, 0x2e, 0x33, 0x34, 0x32 };

    /* taken from Digital Video Processing for Engineers, Suhel Dhanani, Michael Parker */
    constexpr std::array CHROMINANCE_QUANTIZATION_TABLE {
        0x09, 0x09,
        0x09, 0x0c, 0x0b, 0x0c, 0x18, 0x0d, 0x0d, 0x18, 0x32, 0x21, 0x1c, 0x21, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32 };

    ProcessedPixelsType downSample(const std::vector<Util::Pixel4>& rawPixels) {
        ProcessedPixelsType processedPixels{};
        for (int pixelNumber = 0; pixelNumber < rawPixels.size(); pixelNumber++) {
            const Util::Pixel4 pix4 = Util::rgbToYCrCb(rawPixels.at(pixelNumber));
            processedPixels.yValue.push_back(pix4.y);
            if (pixelNumber % (QUANTIZATION_COMPRESSION) == 0) {
                processedPixels.cbValue.push_back(pix4.cb);
                processedPixels.crValue.push_back(pix4.cr);
            }
        }
        return processedPixels;
    }
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

int JFIF::encode(const uint32_t *pixels, const int width, const int height, const int comprQuality) const {
    if (fileDescriptor < 0) {
        std::cerr << "[ERROR] .jfif file could not be created!\n";
        return -1;
    }
    if (lseek(fileDescriptor, SEEK_SET, 0) < 0) {
        std::cerr << "[ERROR] Could not place the descriptor at the beginning of the file!\n";
        return -1;
    }

    if (writeJFIFFile(pixels, width, height) < 0) {
        std::cout << "[Error] Unable to create the .jfif file!\n";
        return -1;
    }
    std::cout << "[INFO] Successfully created the .jfif file!\n";
    return 0;
}

[[nodiscard]] ProcessedPixelsType JFIF::processPixels(const uint32_t *pixels, const int width, const int height) {
    assert(Util::isPowerOf2(width));
    assert(Util::isPowerOf2(height));
    const std::vector<Util::Pixel4> rawPixels = Util::convertToPixelArray(pixels, width, height);

    ProcessedPixelsType processedPixels = downSample(rawPixels);

    applyDCTAndQuantization(processedPixels.yValue, width, height, LUMINANCE_QUANTIZATION_TABLE);
    applyDCTAndQuantization(processedPixels.cbValue, width / QUANTIZATION_COMPRESSION, height / QUANTIZATION_COMPRESSION, CHROMINANCE_QUANTIZATION_TABLE);
    applyDCTAndQuantization(processedPixels.crValue, width / QUANTIZATION_COMPRESSION, height / QUANTIZATION_COMPRESSION, CHROMINANCE_QUANTIZATION_TABLE);

    return processedPixels;
}

void JFIF::applyDCTAndQuantization(std::vector<int> &comp, const unsigned int width, const unsigned int height, const std::array<int, 64> &TABLE) {
    for (int i = 0; i < height; i += BLOCK_SIZE) {
        for (int j = 0; j < width; j += BLOCK_SIZE) {
            std::array<std::array<double, BLOCK_SIZE>, BLOCK_SIZE> block{};
            std::array<unsigned int, BLOCK_SIZE * BLOCK_SIZE> blockIndices{};
            for (int x = 0; x < BLOCK_SIZE; x++) {
                for (int y = 0; y < BLOCK_SIZE; y++) {
                    block[x][y] = comp.at((i + x) * width + (j + y));
                    blockIndices[x * BLOCK_SIZE + y] = (i + x) * width + (j + y);
                }
            }
            Util::Mat8x8 m{block};
            Util::Mat8x8 dctOutput = Util::Mat8x8::dctMat() * m * Util::Mat8x8::dctMat().transpose();

            // process blocks
            for (int x = 0; x < 8; x++)
                for (int y = 0; y < 8; y++)
                    comp.at((i + x) * width + (j + y)) = static_cast<int>(dctOutput.at(x, y) / TABLE[x * BLOCK_SIZE + y]);
        }
    }
}

int JFIF::writeJFIFFile(const uint32_t* pixels, const int width, const int height) const {
    if (writeMarker("SOI") < 0) return -1;
    if (writeAPP0Segment() < 0) return -1;
    // if (writeCOMMarker() < 0) return -1;
    if (writeDQTSegments() < 0) return -1;
    if (writeSOFSegment(width, height) < 0) return -1;
    if (writeDHTSegments() < 0) return -1;
    if (writeSOSSegment() < 0) return -1;
    if (writeCompressedPixelData(pixels, width, height) < 0) return -1;
    if (writeMarker("EOI") < 0) return -1;
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

int JFIF::writeAPP0Segment() const {
    if (writeMarker("APP0") < 0)
        return -1;

    std::vector<uint8_t> segmentData;
    segmentData.insert(segmentData.end(), {'J', 'F', 'I', 'F', '\0'}); // Identifier
    segmentData.insert(segmentData.end(), {0x01, 0x01});               // Version major & minor
    segmentData.push_back(0x01);                                                // Units
    segmentData.insert(segmentData.end(), {0x00, 0x60});               // X density
    segmentData.insert(segmentData.end(), {0x00, 0x60});               // Y density
    segmentData.push_back(0x00);                                                // X thumbnail
    segmentData.push_back(0x00);                                                // Y thumbnail
                                                                                  // no Thumbnail
    return writeSegmentData(segmentData);
}

int JFIF::writeDQTSegments() const {
    if (writeLuminanceDQTMarker() < 0) return -1;
    if (writeChrominanceDQTMarker() < 0) return -1;
    return 0;
}

int JFIF::writeSOFSegment(const int width, const int height) const {
    if (writeMarker("SOF0") < 0) return -1;

    std::vector<uint8_t> segmentData;
    segmentData.push_back(0x08); // 8-bit precision

    std::vector byteHeight = Util::convertIntToTwoBytes(height);
    std::vector byteWidth = Util::convertIntToTwoBytes(width);
    segmentData.insert(segmentData.end(), byteHeight.begin(), byteHeight.end()); // image height
    segmentData.insert(segmentData.end(), byteWidth.begin(), byteWidth.end()); // image width

    segmentData.push_back(0x03); // number of components
    // component ID, horizontal sampling ' vertical sampling, quantization table
    segmentData.insert(segmentData.end(), {0x01, 0b0010'0010, 0x00}); // Y
    segmentData.insert(segmentData.end(), {0x02, 0b0001'0001, 0x01}); // Cb
    segmentData.insert(segmentData.end(), {0x03, 0b0001'0001, 0x01}); // Cr

    return writeSegmentData(segmentData);
}

int JFIF::writeDHTSegments() const {
    std::vector<uint8_t> segmentData{
        0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
        0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
    if (writeMarker("DHT") < 0 || writeSegmentData(segmentData) < 0) return -1;
    segmentData.clear();

    segmentData = {
        0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05,
        0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7d, 0x01, 0x02, 0x03,
        0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13,
        0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1,
        0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0, 0x24,
        0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19,
        0x1a, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36,
        0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
        0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,
        0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74,
        0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86,
        0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
        0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
        0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,
        0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1,
        0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1,
        0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa};
    if (writeMarker("DHT") < 0 || writeSegmentData(segmentData) < 0) return -1;
    segmentData.clear();

    segmentData = {
        0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
        0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
    if (writeMarker("DHT") < 0 || writeSegmentData(segmentData) < 0) return -1;
    segmentData.clear();

    segmentData = {
        0x11, 0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07,
        0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02,
        0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51,
        0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42,
        0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15,
        0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1,
        0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35,
        0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47,
        0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
        0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73,
        0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84,
        0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95,
        0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
        0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
        0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
        0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
        0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa};
    if (writeMarker("DHT") < 0 || writeSegmentData(segmentData) < 0) return -1;

    return 0;
}

int JFIF::writeSOSSegment() const {
    if (writeMarker("SOS") < 0) return -1;

    std::vector<uint8_t> segmentData;
    segmentData.push_back(0x03); // component count
    segmentData.insert(segmentData.end(), {0x01, 0b0000'0000}); // component identifier, DC Huffman table ' AC Huffman table
    segmentData.insert(segmentData.end(), {0x02, 0b0001'0001}); // component identifier, DC Huffman table ' AC Huffman table
    segmentData.insert(segmentData.end(), {0x03, 0b0001'0001}); // component identifier, DC Huffman table ' AC Huffman table
    segmentData.push_back(0x00); // spectral selection start
    segmentData.push_back(0x3F); // spectral selection end
    segmentData.push_back(0b0000'0000); // successive approximation

    return writeSegmentData(segmentData);
}

int JFIF::writeCompressedPixelData(const uint32_t *pixels, const int width, const int height) const {
    const ProcessedPixelsType processedPixels = processPixels(pixels, width, height);
    const std::vector<uint8_t> buffer {
        0xc5, 0x95, 0x8a, 0xc2, 0xec, 0x38, 0x21,
        0x49, 0x14, 0xdb, 0x67, 0x69, 0x2d, 0xd5, 0x98, 0xe5, 0x8e,
        0x72, 0x7f, 0x1a, 0xaf, 0x7d, 0x1a, 0x24, 0x0a,
        0x55, 0x15, 0x4e, 0xee, 0xa0, 0x63, 0xb1, 0xab,
        0x3e, 0x44, 0x5f, 0xf3, 0xc9, 0x3f, 0xef, 0x91,
        0x5f, 0x65, 0x0a, 0xb8, 0x8a, 0x98, 0xf9, 0xa4,
        0x95, 0xa1, 0x14, 0xad, 0xcc, 0xec, 0xdc, 0x9d,
        0xd3, 0xdb, 0x75, 0x6b, 0x6d, 0xd4, 0xfd, 0x8d,
        0xa4, 0xa2, 0x7f};
    if (write(fileDescriptor, buffer.data(), buffer.size()) < 0)
        return -1;
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
    segmentData.push_back(0b0000'0001);  // table identifier ' quantization values: 1 byte unsigned
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
