#include <iostream>

#include "BMP_Decoder/BMP.h"
#include "JFIF_Encoder/JFIF.h"
#include "Util/FreeFunctions.h"

int main(const int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input_filepath> <output_filepath> <optional --verbose>" << std::endl;
        return EXIT_FAILURE;
    }

    BMP bmpImg(argv[1]);
    int status = bmpImg.isOpen();
    if (status < 0) {
        std::cerr << "[ERROR] File \'" << argv[1] << "\' could not be opened!\n";
        return -1;
    }

    status = bmpImg.process();
    if (status < 0) {
        std::cerr << "[ERROR] Exiting program, image of unsupported format!\n";
        return -1;
    }

    if (argc == 4 && strcmp(argv[3], "--verbose") == 0)
        bmpImg.print();

    const uint32_t * pxd = bmpImg.getPixelData();

    const JFIF jfif{argv[2], "result"};

    constexpr int jpegCompressionQuality = 90;
    status = jfif.encode(pxd, bmpImg.getWidth(), bmpImg.getHeight(), jpegCompressionQuality);
    if (status < 0) {
        std::cerr << "[ERROR] Image could not be converted to .jpg\n";
        return -1;
    }

    // debug
    // Util::renderPixels(pxd, bmpImg.getWidth(), bmpImg.getHeight());
    delete[] pxd;
    return 0;
}