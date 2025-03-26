cmake_minimum_required(VERSION 3.30)
project(BMP2JPEG)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)

add_executable(BMP2JPEG src/main.cpp
        src/Core/ImageRenderer.cpp
        src/Core/ImageRenderer.h
        src/BMP_Decoder/BMP.cpp
        src/BMP_Decoder/BMP.h
        src/BMP_Decoder/ColorTable.cpp
        src/BMP_Decoder/ColorTable.h
        src/BMP_Decoder/PixelData.cpp
        src/BMP_Decoder/PixelData.h
        src/BMP_Decoder/FileHeader.cpp
        src/BMP_Decoder/FileHeader.h
        src/BMP_Decoder/DIBHeader.cpp
        src/BMP_Decoder/DIBHeader.h
        src/BMP_Decoder/ExtraBitMask.cpp
        src/BMP_Decoder/ExtraBitMask.h
        src/JFIF_Encoder/JPEG.cpp
        src/JFIF_Encoder/JPEG.h)

target_include_directories(BMP2JPEG PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(BMP2JPEG PRIVATE ${SDL2_LIBRARIES})
