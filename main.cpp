#include "ImageTypes/BMP/BMP.h"
#include "Core/ImageRenderer.h"

int main()
{
    constexpr char BMP_FILE_PATH[] = "Resources/cell.bmp";
    BMP bmpImg(BMP_FILE_PATH);
    if (bmpImg.getFileDescriptor() < 0) {
        std::cerr << "File \'" << bmpImg.getPath() << "\' could not be opened!\n";
        return -1;
    }
    if (bmpImg.process() < 0)
    {
        std::cerr << "Exiting program, image of unsupported format!\n";
        return -1;
    }

    ImageRenderer bmpRenderer{};
    if (bmpRenderer.init() < 0)
    {
        std::cerr << "Renderer failed to init, exiting!\n";
        return -1;
    }
    if (bmpRenderer.initTexture(bmpImg.getWidth(), bmpImg.getHeight(), bmpImg.getPixelData()) < 0)
    {
        std::cerr << "Renderer failed to initialize texture!\n";
        return -1;
    }
    bmpImg.print();
    bmpRenderer.runGameLoop();
    return 0;
}