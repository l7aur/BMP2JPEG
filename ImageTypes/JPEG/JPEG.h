#pragma once

#include "../../Core/Image.h"

class JPEG : public Image
{
public:
    explicit JPEG(const char *filePath) : Image(filePath) {}
    void print() const override {};

private:
};