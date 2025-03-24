#pragma once

class Image
{
public:
    virtual ~Image();
    virtual void print() const = 0;
    virtual int process() = 0;
    [[nodiscard]] int getFileDescriptor() const { return fileDescriptor; };
    [[nodiscard]] const char* getPath() const { return path; };
protected:
    int fileDescriptor{-1};
    const char* path{nullptr};
    static constexpr size_t MAXIMUM_FILE_SIZE_IN_BYTES{500'000}; // roughly a 300x300 image with 4 channels

    explicit Image(const char *filePath);
};