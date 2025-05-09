#include <iostream>

#include "BMP_Decoder/BMP.h"
#include "JFIF_Encoder/JFIF.h"
#include "Util/FreeFunctions.h"

int main(int argc, char *argv[]) {
    constexpr char BMP_FILE_PATH[] = "Resources/cameraman.bmp";
    constexpr int jpegCompressionQuality = 90;

    BMP bmpImg(BMP_FILE_PATH);
    int status = bmpImg.isOpen();
    if (status < 0) {
        std::cerr << "[ERROR] File \'" << BMP_FILE_PATH << "\' could not be opened!\n";
        return -1;
    }

    status = bmpImg.process();
    if (status < 0) {
        std::cerr << "[ERROR] Exiting program, image of unsupported format!\n";
        return -1;
    }

    // bmpImg.print();

    const uint32_t * pxd = bmpImg.getPixelData();

    const JFIF jfif{"./Resources/Converted/", "result"};

    status = jfif.encode(pxd, bmpImg.getWidth(), bmpImg.getHeight(), jpegCompressionQuality);
    if (status < 0) {
        std::cerr << "[ERROR] Image could not be converted to .jfif\n";
        return -1;
    }

    // debug
    Util::renderPixels(pxd, bmpImg.getWidth(), bmpImg.getHeight());
    return 0;
}