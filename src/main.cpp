#include <iostream>

#include "BMP_Decoder/BMP.h"
#include "JFIF_Encoder/JFIF.h"
#include "Util/ImageRenderer.h"

int main(int argc, char *argv[]) {
    constexpr char BMP_FILE_PATH[] = "Resources/saturn.bmp";
    BMP bmpImg(BMP_FILE_PATH);

    int status = bmpImg.getFileDescriptor();
    if (status < 0) {
        std::cerr << "File \'" << bmpImg.getPath() << "\' could not be opened!\n";
        return -1;
    }

    status = bmpImg.process();
    if (status < 0) {
        std::cerr << "Exiting program, image of unsupported format!\n";
        return -1;
    }

    JFIF jfif{"./", "result"};

    status = jfif.encode(bmpImg.getPixelData(), bmpImg.getWidth(), bmpImg.getHeight());
    if (status < 0) {
        std::cerr << "Image could not be converted to .jfif\n";
        return -1;
    }

    // ImageRenderer bmpRenderer{};
    // if (bmpRenderer.init() < 0)
    // {
    //     std::cerr << "Renderer failed to init, exiting!\n";
    //     return -1;
    // }
    // if (bmpRenderer.initTexture(bmpImg.getWidth(), bmpImg.getHeight(), bmpImg.getPixelData()) < 0)
    // {
    //     std::cerr << "Renderer failed to initialize texture!\n";
    //     return -1;
    // }
    // bmpImg.print();
    // bmpRenderer.runGameLoop();
    return 0;
}