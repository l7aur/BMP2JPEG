#include "Image.h"

#include <fcntl.h>
#include <unistd.h>

Image::~Image() {
    close(fileDescriptor);
}

Image::Image(const char *filePath)
    : path(filePath)
{
    fileDescriptor = open(filePath, O_RDONLY);
}
