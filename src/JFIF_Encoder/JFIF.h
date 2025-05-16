#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Util {
    struct Pixel4;
}
struct ProcessedPixelsType;

class JFIF {
public:
    explicit JFIF(const std::string_view& folderPath, const std::string_view& fileName);
    ~JFIF() { cleanup(); }

    void print() const;

    [[nodiscard]]int encode(const uint32_t *pixels, int width, int height, int comprQuality) const;

private:
    int fileDescriptor{-1};

    [[nodiscard]] static ProcessedPixelsType processPixels(const uint32_t *pixels, int width, int height) ;
    static void applyDCTAndQuantization(std::vector<int> &comp, unsigned int width, unsigned int height, const std::array<int, 64> &TABLE) ;

    [[nodiscard]] int writeJFIFFile(const uint32_t* pixels, int width, int height) const;
    [[nodiscard]] int writeMarker(const char* markerId) const;
    [[nodiscard]] int writeAPP0Segment() const;
    [[nodiscard]] int writeDQTSegments() const;
    [[nodiscard]] int writeSOFSegment(int width, int height) const;
    [[nodiscard]] int writeDHTSegments() const;
    [[nodiscard]] int writeSOSSegment() const;
    [[nodiscard]] int writeCompressedPixelData(const uint32_t* pixels, int width, int height) const;
    [[nodiscard]] int writeCOMMarker() const;

    [[nodiscard]] int writeLuminanceDQTMarker() const;
    [[nodiscard]] int writeChrominanceDQTMarker() const;

    [[nodiscard]] int writeSegmentData(const std::vector<uint8_t>& segmentData) const;
    [[nodiscard]] int writeSegmentData(const uint8_t* segmentData, size_t n) const;

    void cleanup() const;
};
