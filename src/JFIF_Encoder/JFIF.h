#pragma once
#include <cstdint>
#include <string>

class JFIF {
public:
    explicit JFIF(const std::string_view& folderPath, const std::string_view& fileName);
    ~JFIF() { cleanup(); }

    [[nodiscard]]int encode(const uint32_t *pixels, int width, int height) const;
    void print() const;

private:
    int fileDescriptor{-1};

    [[nodiscard]] int writeMarker(const char* markerId) const;
    [[nodiscard]] int writeAPP0Header() const;
    [[nodiscard]] int writeDQTMarker() const;
    [[nodiscard]] int writeSegmentData(const std::string_view& segmentData) const;
    void cleanup() const;
};
