#pragma once
#include <cstdint>

class JFIF {
public:
    explicit JFIF(const char *folderPath, const char * fileName);
    ~JFIF() { cleanup(); };
    [[nodiscard]]int encode(const uint32_t *pixels, int width, int height) const;

    void print() const {};

private:
    int fileDescriptor{-1};

    [[nodiscard]] int writeMarker(const char* markerId) const;
    [[nodiscard]] int writeAPP0Header() const;
    void cleanup() const;
};
