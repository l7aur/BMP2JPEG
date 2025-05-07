#pragma once
#include <cstdint>
#include <string>
#include <vector>

class JFIF {
public:
    explicit JFIF(const std::string_view& folderPath, const std::string_view& fileName);
    ~JFIF() { cleanup(); }

    void print() const;

    [[nodiscard]]int encode(const uint32_t *pixels, int width, int height, int comprQuality) const;

private:
    int fileDescriptor{-1};

    [[nodiscard]] int writeMarker(const char* markerId) const;

    [[nodiscard]] int writeAPP0Segment() const;
    [[nodiscard]] int writeDQTSegments() const;
    [[nodiscard]] int writeSOFSegment(int width, int height) const;
    [[nodiscard]] int writeDHTSegment() const;
    [[nodiscard]] int writeSOSSegment() const;
    [[nodiscard]] int writeCOMMarker() const;

    [[nodiscard]] int writeLuminanceDQTMarker() const;
    [[nodiscard]] int writeChrominanceDQTMarker() const;

    [[nodiscard]] int writeSegmentData(const std::vector<uint8_t>& segmentData) const;

    void cleanup() const;
};
