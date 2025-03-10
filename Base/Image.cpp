#include "Image.h"

#include <fcntl.h>
#include <iostream>

Image::Image(const char *filePath)
    : path(filePath)
{
    fileDescriptor = open(filePath, O_RDONLY);
}
