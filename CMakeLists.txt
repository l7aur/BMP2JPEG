cmake_minimum_required(VERSION 3.30)
project(BMP2JPEG)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)

add_executable(BMP2JPEG main.cpp
        Base/Image.cpp
        Base/Image.h
        Base/ImageRenderer.cpp
        Base/ImageRenderer.h
        ImageTypes/BMP/BMP.cpp
        ImageTypes/BMP/BMP.h
        Base/Constants.h
        ImageTypes/BMP/ColorTable.cpp
        ImageTypes/BMP/ColorTable.h
        ImageTypes/BMP/PixelData.cpp
        ImageTypes/BMP/PixelData.h
        ImageTypes/BMP/FileHeader.cpp
        ImageTypes/BMP/FileHeader.h
        ImageTypes/BMP/DIBHeader.cpp
        ImageTypes/BMP/DIBHeader.h
        ImageTypes/BMP/ExtraBitMask.cpp
        ImageTypes/BMP/ExtraBitMask.h
        ImageTypes/JPEG/JPEG.cpp
        ImageTypes/JPEG/JPEG.h)

target_include_directories(BMP2JPEG PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(BMP2JPEG PRIVATE ${SDL2_LIBRARIES})
