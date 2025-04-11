#include <iostream>

#include "BMP_Decoder/BMP.h"
#include "JFIF_Encoder/JFIF.h"
#include "Util/FreeFunctions.h"

int main(int argc, char *argv[]) {
    constexpr char BMP_FILE_PATH[] = "Resources/flowers_24bits.bmp";
    BMP bmpImg(BMP_FILE_PATH);

    int status = bmpImg.getFileDescriptor();
    if (status < 0) {
        std::cerr << "[ERROR] File \'" << bmpImg.getPath() << "\' could not be opened!\n";
        return -1;
    }

    status = bmpImg.process();
    if (status < 0) {
        std::cerr << "[ERROR] Exiting program, image of unsupported format!\n";
        return -1;
    }

    bmpImg.print();

    // const JFIF jfif{"./", "result"};
    //
    // status = jfif.encode(bmpImg.getPixelData(), bmpImg.getWidth(), bmpImg.getHeight());
    // if (status < 0) {
    //     std::cerr << "[ERROR] Image could not be converted to .jfif\n";
    //     return -1;
    // }

    Util::renderPixels(bmpImg.getWidth(), bmpImg.getHeight(), bmpImg.getPixelData());

    return 0;
}