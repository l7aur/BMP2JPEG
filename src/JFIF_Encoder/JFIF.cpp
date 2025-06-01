#include "JFIF.h"

#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <map>
#include <array>
#include <cassert>
#include <bitset>

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
    constexpr unsigned int QUANTIZATION_COMPRESSION{ 1 };
    constexpr int BLOCK_SIZE{ 8 };
    constexpr int L{ 256 };
    std::vector<bool> compressedData {};
    int dcPrevCoefficient = 0;

    const std::map<const std::string, const marker> MARKERS {
        {"SOI", 0xD8'FF}, {"EOI", 0xD9'FF}, {"SOF0", 0xC0'FF}, {"DHT", 0xC4'FF},
        {"SOS", 0xDA'FF}, {"DQT", 0xDB'FF}, {"APP0", 0xE0'FF},{"COM", 0xFE'FF} };

    constexpr std::array LUMINANCE_QUANTIZATION_TABLE {
        0x08, 0x06, 0x06, 0x07, 0x06, 0x05, 0x08, 0x07, 0x07, 0x07, 0x09, 0x09, 0x08, 0x0a, 0x0c, 0x14,
        0x0d, 0x0c, 0x0b, 0x0b, 0x0c, 0x19, 0x12, 0x13, 0x0f, 0x14, 0x1d, 0x1a, 0x1f, 0x1e, 0x1d, 0x1a,
        0x1c, 0x1c, 0x20, 0x24, 0x2e, 0x27, 0x20, 0x22, 0x2c, 0x23, 0x1c, 0x1c, 0x28, 0x37, 0x29, 0x2c,
        0x30, 0x31, 0x34, 0x34, 0x34, 0x1f, 0x27, 0x39, 0x3d, 0x38, 0x32, 0x3c, 0x2e, 0x33, 0x34, 0x32 };

    constexpr std::array CHROMINANCE_QUANTIZATION_TABLE {
        0x09, 0x09, 0x09, 0x0c, 0x0b, 0x0c, 0x18, 0x0d, 0x0d, 0x18, 0x32, 0x21, 0x1c, 0x21, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32,
        0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32, 0x32 };

    const std::map<const unsigned int, const std::vector<bool>> BITS_TO_HUFFMAN_DC {
        {0, {false, false}},        // 00
        {1, {false, true, false}},  // 010
        {2, {false, true, true}},   // 011
        {3, {true, false, false}},  // 100
        {4, {true, false, true}},   // 101
        {5, {true, true, false}},   // 110
        {6, {true, true, true, false}}, //1110
        {7, {true, true, true, true, false}}, // 11110
        {8, {true, true, true, true, true, false}}, // 111110
        {9, {true, true, true, true, true, true, false}}, // 1111110
        {10, {true, true, true, true, true, true, true, false}}, // 111'111'10
        {11, {true, true, true, true, true, true, true, true, false}} }; // 111'111'110

    const std::map<std::pair<int, int>, const std::vector<bool>> RL_BITS_TO_HUFFMAN_AC {
        {{0, 0}, {true,false,true,false}},
        {{0, 1}, {false,false}},
        {{0, 2}, {false,true}},
        {{0, 3}, {true,false,false}},
        {{0, 4}, {true,false,true,true}},
        {{0, 5}, {true,true,false,true,false}},
        {{0, 6}, {true,true,true,true,false,false,false}},
        {{0, 7}, {true,true,true,true,true,false,false,false}},
        {{0, 8}, {true,true,true,true,true,true,false,true,true,false}},
        {{0, 9}, {true,true,true,true,true,true,true,true,true,false,false,false,false,false,true,false}},
        {{0, 10}, {true,true,true,true,true,true,true,true,true,false,false,false,false,false,true,true}},

        {{1, 1}, {true,true,false,false}},
        {{1, 2}, {true,true,false,true,true}},
        {{1, 3}, {true,true,true,true,false,false,true}},
        {{1, 4}, {true,true,true,true,true,false,true,true,false}},
        {{1, 5}, {true,true,true,true,true,true,true,false,true,true,false}},
        {{1, 6}, {true,true,true,true,true,true,true,true,true,false,false,false,false,true,false,false}},
        {{1, 7}, {true,true,true,true,true,true,true,true,true,false,false,false,false,true,false,true}},
        {{1, 8}, {true,true,true,true,true,true,true,true,true,false,false,false,false,true,true,false}},
        {{1, 9}, {true,true,true,true,true,true,true,true,true,false,false,false,false,true,true,true}},
        {{1, 10}, {true,true,true,true,true,true,true,true,true,false,false,false,true,false,false,false}},

        {{2, 1}, {true,true,true,false,false}},
        {{2, 2}, {true,true,true,true,true,false,false,true}},
        {{2, 3}, {true,true,true,true,true,true,false,true,true,true}},
        {{2, 4}, {true,true,true,true,true,true,true,true,false,true,false,false}},
        {{2, 5}, {true,true,true,true,true,true,true,true,true,false,false,false,true,false,false,true}},
        {{2, 6}, {true,true,true,true,true,true,true,true,true,false,false,false,true,false,true,false}},
        {{2, 7}, {true,true,true,true,true,true,true,true,true,false,false,false,true,false,true,true}},
        {{2, 8}, {true,true,true,true,true,true,true,true,true,false,false,false,true,true,false,false}},
        {{2, 9}, {true,true,true,true,true,true,true,true,true,false,false,false,true,true,false,true}},
        {{2, 10}, {true,true,true,true,true,true,true,true,true,false,false,false,true,true,true,false}},

        {{3, 1}, {true,true,true,false,true,false}},
        {{3, 2}, {true,true,true,true,true,false,true,true,true}},
        {{3, 3}, {true,true,true,true,true,true,true,true,false,true,false,true}},
        {{3, 4}, {true,true,true,true,true,true,true,true,true,false,false,false,true,true,true,true}},
        {{3, 5}, {true,true,true,true,true,true,true,true,true,false,false,true,false,false,false,false}},
        {{3, 6}, {true,true,true,true,true,true,true,true,true,false,false,true,false,false,false,true}},
        {{3, 7}, {true,true,true,true,true,true,true,true,true,false,false,true,false,false,true,false}},
        {{3, 8}, {true,true,true,true,true,true,true,true,true,false,false,true,false,false,true,true}},
        {{3, 9}, {true,true,true,true,true,true,true,true,true,false,false,true,false,true,false,false}},
        {{3, 10}, {true,true,true,true,true,true,true,true,true,false,false,true,false,true,false,true}},

        {{4, 1}, {true,true,true,false,true,true}},
        {{4, 2}, {true,true,true,true,true,true,true,false,false,false}},
        {{4, 3}, {true,true,true,true,true,true,true,true,true,false,false,true,false,true,true,false}},
        {{4, 4}, {true,true,true,true,true,true,true,true,true,false,false,true,false,true,true,true}},
        {{4, 5}, {true,true,true,true,true,true,true,true,true,false,false,true,true,false,false,false}},
        {{4, 6}, {true,true,true,true,true,true,true,true,true,false,false,true,true,false,false,true}},
        {{4, 7}, {true,true,true,true,true,true,true,true,true,false,false,true,true,false,true,false}},
        {{4, 8}, {true,true,true,true,true,true,true,true,true,false,false,true,true,false,true,true}},
        {{4, 9}, {true,true,true,true,true,true,true,true,true,false,false,true,true,true,false,false}},
        {{4, 10}, {true,true,true,true,true,true,true,true,true,false,false,true,true,true,false,true}},

        {{5, 1}, {true,true,true,true,false,true,false}},
        {{5, 2}, {true,true,true,true,true,true,true,false,true,true,true}},
        {{5, 3}, {true,true,true,true,true,true,true,true,true,false,false,true,true,true,true,false}},
        {{5, 4}, {true,true,true,true,true,true,true,true,true,false,false,true,true,true,true,true}},
        {{5, 5}, {true,true,true,true,true,true,true,true,true,false,true,false,false,false,false,false}},
        {{5, 6}, {true,true,true,true,true,true,true,true,true,false,true,false,false,false,false,true}},
        {{5, 7}, {true,true,true,true,true,true,true,true,true,false,true,false,false,false,true,false}},
        {{5, 8}, {true,true,true,true,true,true,true,true,true,false,true,false,false,false,true,true}},
        {{5, 9}, {true,true,true,true,true,true,true,true,true,false,true,false,false,true,false,false}},
        {{5, 10}, {true,true,true,true,true,true,true,true,true,false,true,false,false,true,false,true}},

        {{6, 1}, {true,true,true,true,false,true,true}},
        {{6, 2}, {true,true,true,true,true,true,true,true,false,true,true,false}},
        {{6, 3}, {true,true,true,true,true,true,true,true,true,false,true,false,false,true,true,false}},
        {{6, 4}, {true,true,true,true,true,true,true,true,true,false,true,false,false,true,true,true}},
        {{6, 5}, {true,true,true,true,true,true,true,true,true,false,true,false,true,false,false,false}},
        {{6, 6}, {true,true,true,true,true,true,true,true,true,false,true,false,true,false,false,true}},
        {{6, 7}, {true,true,true,true,true,true,true,true,true,false,true,false,true,false,true,false}},
        {{6, 8}, {true,true,true,true,true,true,true,true,true,false,true,false,true,false,true,true}},
        {{6, 9}, {true,true,true,true,true,true,true,true,true,false,true,false,true,true,false,false}},
        {{6, 10}, {true,true,true,true,true,true,true,true,true,false,true,false,true,true,false,true}},

        {{7, 1}, {true,true,true,true,true,false,true,false}},
        {{7, 2}, {true,true,true,true,true,true,true,true,false,true,true,true}},
        {{7, 3}, {true,true,true,true,true,true,true,true,true,false,true,false,true,true,true,false}},
        {{7, 4}, {true,true,true,true,true,true,true,true,true,false,true,false,true,true,true,true}},
        {{7, 5}, {true,true,true,true,true,true,true,true,true,false,true,true,false,false,false,false}},
        {{7, 6}, {true,true,true,true,true,true,true,true,true,false,true,true,false,false,false,true}},
        {{7, 7}, {true,true,true,true,true,true,true,true,true,false,true,true,false,false,true,false}},
        {{7, 8}, {true,true,true,true,true,true,true,true,true,false,true,true,false,false,true,true}},
        {{7, 9}, {true,true,true,true,true,true,true,true,true,false,true,true,false,true,false,false}},
        {{7, 10}, {true,true,true,true,true,true,true,true,true,false,true,true,false,true,false,true}},

        {{8, 1}, {true,true,true,true,true,true,false,false,false}},
        {{8, 2}, {true,true,true,true,true,true,true,true,true,false,false,false,false,false,false}},
        {{8, 3}, {true,true,true,true,true,true,true,true,true,false,true,true,false,true,true,false}},
        {{8, 4}, {true,true,true,true,true,true,true,true,true,false,true,true,false,true,true,true}},
        {{8, 5}, {true,true,true,true,true,true,true,true,true,false,true,true,true,false,false,false}},
        {{8, 6}, {true,true,true,true,true,true,true,true,true,false,true,true,true,false,false,true}},
        {{8, 7}, {true,true,true,true,true,true,true,true,true,false,true,true,true,false,true,false}},
        {{8, 8}, {true,true,true,true,true,true,true,true,true,false,true,true,true,false,true,true}},
        {{8, 9}, {true,true,true,true,true,true,true,true,true,false,true,true,true,true,false,false}},
        {{8, 10}, {true,true,true,true,true,true,true,true,true,false,true,true,true,true,false,true}},

        {{9, 1}, {true,true,true,true,true,true,false,false,true}},
        {{9, 2}, {true,true,true,true,true,true,true,true,true,false,true,true,true,true,true,false}},
        {{9, 3}, {true,true,true,true,true,true,true,true,true,false,true,true,true,true,true,true}},
        {{9, 4}, {true,true,true,true,true,true,true,true,true,true,false,false,false,false,false,false}},
        {{9, 5}, {true,true,true,true,true,true,true,true,true,true,false,false,false,false,false,true}},
        {{9, 6}, {true,true,true,true,true,true,true,true,true,true,false,false,false,false,true,false}},
        {{9, 7}, {true,true,true,true,true,true,true,true,true,true,false,false,false,false,true,true}},
        {{9, 8}, {true,true,true,true,true,true,true,true,true,true,false,false,false,true,false,false}},
        {{9, 9}, {true,true,true,true,true,true,true,true,true,true,false,false,false,true,false,true}},
        {{9, 10}, {true,true,true,true,true,true,true,true,true,true,false,false,false,true,true,false}},

        {{10, 1}, {true,true,true,true,true,true,false,true,false}},
        {{10, 2}, {true,true,true,true,true,true,true,true,true,true,false,false,false,true,true,true}},
        {{10, 3}, {true,true,true,true,true,true,true,true,true,true,false,false,true,false,false,false}},
        {{10, 4}, {true,true,true,true,true,true,true,true,true,true,false,false,true,false,false,true}},
        {{10, 5}, {true,true,true,true,true,true,true,true,true,true,false,false,true,false,true,false}},
        {{10, 6}, {true,true,true,true,true,true,true,true,true,true,false,false,true,false,true,true}},
        {{10, 7}, {true,true,true,true,true,true,true,true,true,true,false,false,true,true,false,false}},
        {{10, 8}, {true,true,true,true,true,true,true,true,true,true,false,false,true,true,false,true}},
        {{10, 9}, {true,true,true,true,true,true,true,true,true,true,false,false,true,true,true,false}},
        {{10, 10}, {true,true,true,true,true,true,true,true,true,true,false,false,true,true,true,true}},

        {{11, 1}, {true,true,true,true,true,true,true,false,false,true}},
        {{11, 2}, {true,true,true,true,true,true,true,true,true,true,false,true,false,false,false,false}},
        {{11, 3}, {true,true,true,true,true,true,true,true,true,true,false,true,false,false,false,true}},
        {{11, 4}, {true,true,true,true,true,true,true,true,true,true,false,true,false,false,true,false}},
        {{11, 5}, {true,true,true,true,true,true,true,true,true,true,false,true,false,false,true,true}},
        {{11, 6}, {true,true,true,true,true,true,true,true,true,true,false,true,false,true,false,false}},
        {{11, 7}, {true,true,true,true,true,true,true,true,true,true,false,true,false,true,false,true}},
        {{11, 8}, {true,true,true,true,true,true,true,true,true,true,false,true,false,true,true,false}},
        {{11, 9}, {true,true,true,true,true,true,true,true,true,true,false,true,false,true,true,true}},
        {{11, 10}, {true,true,true,true,true,true,true,true,true,true,false,true,true,false,false,false}},

        {{12, 1}, {true,true,true,true,true,true,true,false,true,false}},
        {{12, 2}, {true,true,true,true,true,true,true,true,true,true,false,true,true,false,false,true}},
        {{12, 3}, {true,true,true,true,true,true,true,true,true,true,false,true,true,false,true,false}},
        {{12, 4}, {true,true,true,true,true,true,true,true,true,true,false,true,true,false,true,true}},
        {{12, 5}, {true,true,true,true,true,true,true,true,true,true,false,true,true,true,false,false}},
        {{12, 6}, {true,true,true,true,true,true,true,true,true,true,false,true,true,true,false,true}},
        {{12, 7}, {true,true,true,true,true,true,true,true,true,true,false,true,true,true,true,false}},
        {{12, 8}, {true,true,true,true,true,true,true,true,true,true,false,true,true,true,true,true}},
        {{12, 9}, {true,true,true,true,true,true,true,true,true,true,true,false,false,false,false,false}},
        {{12, 10}, {true,true,true,true,true,true,true,true,true,true,true,false,false,false,false,true}},

        {{13, 1}, {true,true,true,true,true,true,true,true,false,false,false}},
        {{13, 2}, {true,true,true,true,true,true,true,true,true,true,true,false,false,false,true,false}},
        {{13, 3}, {true,true,true,true,true,true,true,true,true,true,true,false,false,false,true,true}},
        {{13, 4}, {true,true,true,true,true,true,true,true,true,true,true,false,false,true,false,false}},
        {{13, 5}, {true,true,true,true,true,true,true,true,true,true,true,false,false,true,false,true}},
        {{13, 6}, {true,true,true,true,true,true,true,true,true,true,true,false,false,true,true,false}},
        {{13, 7}, {true,true,true,true,true,true,true,true,true,true,true,false,false,true,true,true}},
        {{13, 8}, {true,true,true,true,true,true,true,true,true,true,true,false,true,false,false,false}},
        {{13, 9}, {true,true,true,true,true,true,true,true,true,true,true,false,true,false,false,true}},
        {{13, 10}, {true,true,true,true,true,true,true,true,true,true,true,false,true,false,true,false}},

        {{14, 1}, {true,true,true,true,true,true,true,true,true,true,true,false,true,false,true,true}},
        {{14, 2}, {true,true,true,true,true,true,true,true,true,true,true,false,true,true,false,false}},
        {{14, 3}, {true,true,true,true,true,true,true,true,true,true,true,false,true,true,false,true}},
        {{14, 4}, {true,true,true,true,true,true,true,true,true,true,true,false,true,true,true,false}},
        {{14, 5}, {true,true,true,true,true,true,true,true,true,true,true,false,true,true,true,true}},
        {{14, 6}, {true,true,true,true,true,true,true,true,true,true,true,true,false,false,false,false}},
        {{14, 7}, {true,true,true,true,true,true,true,true,true,true,true,true,false,false,false,true}},
        {{14, 8}, {true,true,true,true,true,true,true,true,true,true,true,true,false,false,true,false}},
        {{14, 9}, {true,true,true,true,true,true,true,true,true,true,true,true,false,false,true,true}},
        {{14, 10}, {true,true,true,true,true,true,true,true,true,true,true,true,false,true,false,false}},

        {{15, 0}, {true,true,true,true,true,true,true,true,false,false,true}},
        {{15, 1}, {true,true,true,true,true,true,true,true,true,true,true,true,false,true,false,true}},
        {{15, 2}, {true,true,true,true,true,true,true,true,true,true,true,true,false,true,true,false}},
        {{15, 3}, {true,true,true,true,true,true,true,true,true,true,true,true,false,true,true,true}},
        {{15, 4}, {true,true,true,true,true,true,true,true,true,true,true,true,true,false,false,false}},
        {{15, 5}, {true,true,true,true,true,true,true,true,true,true,true,true,true,false,false,true}},
        {{15, 6}, {true,true,true,true,true,true,true,true,true,true,true,true,true,false,true,false}},
        {{15, 7}, {true,true,true,true,true,true,true,true,true,true,true,true,true,false,true,true}},
        {{15, 8}, {true,true,true,true,true,true,true,true,true,true,true,true,true,true,false,false}},
        {{15, 9}, {true,true,true,true,true,true,true,true,true,true,true,true,true,true,false,true}},
        {{15, 10}, {true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,false}}
    };

    constexpr std::array<uint8_t, 29> HUFFMAN_TABLE_DC_LUMINANCE{
        0x00,
        0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B
    };
    constexpr std::array<uint8_t, 179> HUFFMAN_TABLE_AC_LUMINANCE{
        0x10,
        0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,

        0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
        0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0,
        0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
        0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
        0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
        0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
        0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
        0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
        0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
        0xF9, 0xFA
    };
    constexpr std::array<uint8_t, 179> HUFFMAN_TABLE_AC_CHROMINANCE = HUFFMAN_TABLE_AC_LUMINANCE;
    constexpr std::array<uint8_t, 29> HUFFMAN_TABLE_DC_CHROMINANCE = HUFFMAN_TABLE_DC_LUMINANCE;

    constexpr std::array order {
        std::make_pair(0, 0), std::make_pair(0, 1), std::make_pair(1, 0), std::make_pair(2, 0), std::make_pair(1, 1), std::make_pair(0, 2), std::make_pair(0, 3), std::make_pair(1, 2),
        std::make_pair(2, 1), std::make_pair(3, 0), std::make_pair(4, 0), std::make_pair(3, 1), std::make_pair(2, 2), std::make_pair(1, 3), std::make_pair(0, 4), std::make_pair(0, 5),
        std::make_pair(1, 4), std::make_pair(2, 3), std::make_pair(3, 2), std::make_pair(4, 1), std::make_pair(5, 0), std::make_pair(6, 0), std::make_pair(5, 1), std::make_pair(4, 2),
        std::make_pair(3, 3), std::make_pair(2, 4), std::make_pair(1, 5), std::make_pair(0, 6), std::make_pair(0, 7), std::make_pair(1, 6), std::make_pair(2, 5), std::make_pair(3, 4),
        std::make_pair(4, 3), std::make_pair(5, 2), std::make_pair(6, 1), std::make_pair(7, 0), std::make_pair(7, 1), std::make_pair(6, 2), std::make_pair(5, 3), std::make_pair(4, 4),
        std::make_pair(3, 5), std::make_pair(2, 6), std::make_pair(1, 7), std::make_pair(2, 7), std::make_pair(3, 6), std::make_pair(4, 5), std::make_pair(5, 4), std::make_pair(6, 3),
        std::make_pair(7, 2), std::make_pair(7, 3), std::make_pair(6, 4), std::make_pair(5, 5), std::make_pair(4, 6), std::make_pair(3, 7), std::make_pair(4, 7), std::make_pair(5, 6),
        std::make_pair(6, 5), std::make_pair(7, 4), std::make_pair(7, 5), std::make_pair(6, 6), std::make_pair(5, 7), std::make_pair(6, 7), std::make_pair(7, 6), std::make_pair(7, 7)
    };

    ProcessedPixelsType downSample(const std::vector<Util::Pixel4>& rawPixels) {
        ProcessedPixelsType processedPixels{};
        for (int pixelNumber = 0; pixelNumber < rawPixels.size(); pixelNumber++) {
            const Util::Pixel4 pix4 = Util::rgbToYCrCb(rawPixels.at(pixelNumber));
            processedPixels.yValue.push_back(pix4.y - L / 2);
            if (pixelNumber % (QUANTIZATION_COMPRESSION) == 0) {
                processedPixels.cbValue.push_back(pix4.cb - L / 2);
                processedPixels.crValue.push_back(pix4.cr - L / 2);
            }
        }
        return processedPixels;
    }

    std::pair<std::vector<bool>, unsigned int> getAmplitudeAndBits(int value) {
        if (value == 0) { // Special case for 0 (no bits for amplitude)
            return {std::vector<bool>{}, 0};
        }

        unsigned int bits = 0;
        int tempValue = std::abs(value); // Use absolute value to determine category (number of bits)
        // Determine the 'bits' (category size)
        // This part should be robust:
        if (tempValue >= 1 && tempValue <= 1) bits = 1;
        else if (tempValue >= 2 && tempValue <= 3) bits = 2;
        else if (tempValue >= 4 && tempValue <= 7) bits = 3;
        else if (tempValue >= 8 && tempValue <= 15) bits = 4;
        else if (tempValue >= 16 && tempValue <= 31) bits = 5;
        else if (tempValue >= 32 && tempValue <= 63) bits = 6;
        else if (tempValue >= 64 && tempValue <= 127) bits = 7;
        else if (tempValue >= 128 && tempValue <= 255) bits = 8;
        else if (tempValue >= 256 && tempValue <= 511) bits = 9;
        else if (tempValue >= 512 && tempValue <= 1023) bits = 10;
        else if (tempValue >= 1024 && tempValue <= 2047) bits = 11;

        int encodedAmplitude = value;
        if (value < 0) {
            encodedAmplitude = (value - 1);
        }

        std::vector<bool> amplitudeBits;
        for (int i = static_cast<int>(bits) - 1; i >= 0; --i) {
            amplitudeBits.push_back((encodedAmplitude >> i) & 1);
        }

        return std::make_pair(amplitudeBits, bits);
    }
}

JFIF::JFIF(const std::string_view& folderPath, const std::string_view& fileName) {
    std::string filePathStr = folderPath.data();
    filePathStr.append("/");
    filePathStr.append(fileName);
    filePathStr.append(".jpg");
    fileDescriptor = creat(filePathStr.c_str(),  O_CREAT | S_IRUSR | S_IWUSR);
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
        std::cout << "[Error] Unable to create the .jpg file!\n";
        return -1;
    }
    std::cout << "[INFO] Successfully created the .jpg file!\n";
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
                    comp.at((i + x) * width + (j + y)) = static_cast<int>(std::round(dctOutput.at(x, y) / static_cast<double>(TABLE[x * BLOCK_SIZE + y])));

            // for (int x = 0; x < 8; x++, std::cout << std::endl)
            //     for (int y = 0; y < 8; y++)
            //         std::cout << comp.at((i+x) *width + y+j) << ' ';
            // std::cout << std::endl;

        }
    }
}

int JFIF::writeJFIFFile(const uint32_t* pixels, const int width, const int height) const {
    if (writeMarker("SOI") < 0) return -1;
    if (writeAPP0Segment() < 0) return -1;
    if (writeCOMMarker() < 0) return -1;
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
    catch (...) {
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
    // segmentData.insert(segmentData.end(), {0x01, 0b0010'0010, 0x00}); // Y - 2:1
    segmentData.insert(segmentData.end(), {0x01, 0b0001'0001, 0x00}); // Y
    segmentData.insert(segmentData.end(), {0x02, 0b0001'0001, 0x01}); // Cb
    segmentData.insert(segmentData.end(), {0x03, 0b0001'0001, 0x01}); // Cr

    return writeSegmentData(segmentData);
}

int JFIF::writeDHTSegments() const {

    if (writeMarker("DHT") < 0 || writeSegmentData(HUFFMAN_TABLE_DC_LUMINANCE.data(), HUFFMAN_TABLE_DC_LUMINANCE.size()) < 0) return -1;
    if (writeMarker("DHT") < 0 || writeSegmentData(HUFFMAN_TABLE_DC_CHROMINANCE.data(), HUFFMAN_TABLE_DC_LUMINANCE.size()) < 0) return -1;
    if (writeMarker("DHT") < 0 || writeSegmentData(HUFFMAN_TABLE_AC_LUMINANCE.data(), HUFFMAN_TABLE_AC_LUMINANCE.size()) < 0) return -1;
    if (writeMarker("DHT") < 0 || writeSegmentData(HUFFMAN_TABLE_AC_CHROMINANCE.data(), HUFFMAN_TABLE_AC_LUMINANCE.size()) < 0) return -1;
    return 0;
}

int JFIF::writeSOSSegment() const {
    if (writeMarker("SOS") < 0) return -1;

    std::vector<uint8_t> segmentData;
    segmentData.push_back(0x03); // component count
    segmentData.insert(segmentData.end(), {0x01, 0x00}); // component identifier, DC Huffman table ' AC Huffman table
    segmentData.insert(segmentData.end(), {0x02, 0x00}); // component identifier, DC Huffman table ' AC Huffman table
    segmentData.insert(segmentData.end(), {0x03, 0x00}); // component identifier, DC Huffman table ' AC Huffman table
    segmentData.push_back(0x00); // spectral selection start
    segmentData.push_back(0x3F); // spectral selection end
    segmentData.push_back(0b0000'0000); // successive approximation

    return writeSegmentData(segmentData);
}

int JFIF::writeCompressedPixelData(const uint32_t *pixels, const int width, const int height) const {
    auto [yValue, cbValue, crValue] = processPixels(pixels, width, height);
    dcPrevCoefficient = 0;
    reorder(yValue, width, height);
    dcPrevCoefficient = 0;
    reorder(cbValue, width / QUANTIZATION_COMPRESSION, height / QUANTIZATION_COMPRESSION);
    dcPrevCoefficient = 0;
    reorder(crValue, width / QUANTIZATION_COMPRESSION, height / QUANTIZATION_COMPRESSION);

    static constexpr int BYTE_TO_BITS = 8;
    std::vector<uint8_t> buffer{};
        // 0xfc, 0x5f, 0xa2, 0x8a, 0x2b, 0xfc, 0xa7, 0x3f, 0xef, 0xe0};
    for (int i = 0; i < compressedData.size(); i += BYTE_TO_BITS) {
        uint8_t number = 0x00;
        for (int j = 0; j < BYTE_TO_BITS && i + j < compressedData.size(); j++) {
            number <<= 1;
            number |= static_cast<uint8_t>(compressedData.at(i + j));
        }
        buffer.push_back(number);
        if (number == 0xFF)
            buffer.push_back(0x00);
    }
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

void JFIF::reorder(const std::vector<int>& comp, const unsigned int width, const unsigned int height) {
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

            const Util::Mat8x8 m{block};

            // const Util::Mat8x8 m {
            //     {3, 0, 0, 0, 0, 0, 0, 0},
            //     {0, 0, 0, 0, 0, 0, 0, 0},
            //     {0, 0, 0, 0, 0, 0, 0, 0},
            //     {0, 0, 0, 0, 0, 0, 0, 0},
            //     {0, 0, 0, 0, 0, 0, 0, 0},
            //     {0, 0, 0, 0, 0, 9, 0, 0},
            //     {0, 0, 0, 0, 0, 0, 0, 0},
            //     {0, 0, 0, 0, 0, 0, 0, 0} };
            int currentDCValue = static_cast<int>(m.at(order.at(0).first, order.at(0).second)) - dcPrevCoefficient;
            const auto [amplitude, bits] = getAmplitudeAndBits(currentDCValue);
            auto dcVLI = BITS_TO_HUFFMAN_DC.at(bits);
            compressedData.insert(compressedData.end(), dcVLI.begin(), dcVLI.end());
            compressedData.insert(compressedData.end(), amplitude.begin() + (static_cast<int>(amplitude.size()) - bits), amplitude.end());
            dcPrevCoefficient = currentDCValue;

            // for (auto k : dcVLI)
            //     std::cout << k << ' ';
            // std::cout << std::endl;
            // for (auto k = amplitude.begin() + amplitude.size() - bits; k != amplitude.end(); ++k) {
            //     std::cout << *k << " ";
            // }
            // std::cout << std::endl;

            int lastPositionBeforeAllZeroes = BLOCK_SIZE * BLOCK_SIZE - 1;
            while (true) {
                auto [x_pos, y_pos] = order.at(lastPositionBeforeAllZeroes);
                if (m.at(x_pos, y_pos) == 0 && lastPositionBeforeAllZeroes > 0)
                    lastPositionBeforeAllZeroes--;
                else
                    break;
            }

            int zeroCounter = 0;
            for (int index = 1; index <= lastPositionBeforeAllZeroes; index++) {
                auto [x_pos, y_pos] = order.at(index);
                int currentACValue = static_cast<int>(m.at(x_pos, y_pos));
                if (currentACValue == 0 && zeroCounter < 15) {
                    zeroCounter++;
                    continue;
                }
                const auto [encodingVLI, bits] = getAmplitudeAndBits(currentACValue);
                const auto encodingVLC = RL_BITS_TO_HUFFMAN_AC.at({zeroCounter, bits});
                // for (auto k : encodingVLC) {
                //     std::cout << k << " ";
                // }
                // std::cout << std::endl;

                compressedData.insert(compressedData.end(), encodingVLC.begin(), encodingVLC.end());
                zeroCounter = 0;

                if (currentACValue == 0)
                    continue;

                // for (auto k : encodingVLI) {
                //     std::cout << k << " ";
                // }
                // std::cout << std::endl;

                compressedData.insert(compressedData.end(), encodingVLI.begin(), encodingVLI.end());
            }
            const auto encoding = RL_BITS_TO_HUFFMAN_AC.at({0, 0});
            compressedData.insert(compressedData.end(), encoding.begin(), encoding.end());
        }
    }

}

int JFIF::writeSegmentData(const std::vector<uint8_t>& segmentData) const {
    return writeSegmentData(segmentData.data(), segmentData.size());
}

int JFIF::writeSegmentData(const uint8_t* segmentData, const size_t n) const {
    static constexpr int MARKER_LENGTH_SIZE = 2;
    const uint16_t segmentSize = n + MARKER_LENGTH_SIZE;
    const uint8_t sizeBuffer[MARKER_LENGTH_SIZE] = { static_cast<uint8_t>((segmentSize >> 8) & 0xFF), static_cast<uint8_t>(segmentSize & 0xFF) };

    if (write(fileDescriptor, sizeBuffer, sizeof(sizeBuffer)) < 0)
        return -1;
    if (write(fileDescriptor, segmentData, n) < 0)
        return -1;

    return 0;
}

void JFIF::cleanup() const {
    if (close(fileDescriptor) < 0)
        std::cerr << "[ERROR] Could not close the .jfif file!\n";
}
