# Converting BMP files into JPEG

Project for image processing course

## The BMP format

The BMP file format is a raster graphics image file format used to store bitmap digital images independent of the display device. It is capable of storing bidimensional images in various color depths, and optionally, with data compression, alpha channel and color profile.

### BMP file structure

1. **header**: 14 bytes
- signature: 2 bytes - "BM"
- file size: 4 bytes (size of file in bytes)
- reserved: 4 bytes (when 0 -> unused)
- data offset: 4 bytes (file offset to raster data)

2. **(DIB) info header**: 12 to 124 bytes depending on the version*
- size (of info header)
- width (of image)
- height (of image)
- planes ( = 1)
- bit count (bits per pixel)
    - 1 = monochrome palette => number of colors = 1
    - 4 = 4-bit palette => number of colors = 16
    - 8 = 8-bit palette => number of colors = 256
    - 16 = 16-bit RGB => number of colors = 65536
    - 24 = 24-bit RGB => number of colors = 16M
- compression
    - 0 = BI_RGB => no compression
    - 1 = BI_RLE8 => 8-bit RLE encoding
    - 2 = BI_RLE4 => 4-bit RLE encoding
- image size
- X pixels per meter
- Y pixels per meter
- used colors
- importand colors
- color table (only if bit count <= 8)
    - red
    - green
    - blue
    - reserved (0 = unused)
- ...

3. **raster data**

*) There are 7 possible versions: BITMAPCOREHEADER / OS21XBITMAPHEADER (12 bytes), OS22XBITMAPHEADER (2 types - 64 and 16 bytes), BITMAPINFOHEADER (40 bytes), BITMAPV2INFOHEADER (52 bytes), BITMAPV3INFOHEADER (56 bytes), BITMAPV4HEADER (108 bytes), BITMAPV5HEADER (124 bytes).

## The JPEG format

## Bibliography

1. [BMP GitHub summary](https://gibberlings3.github.io/iesdp/file_formats/ie_formats/bmp.htm#RasterData)
2. [BMP Wiki](https://en.wikipedia.org/wiki/BMP_file_format#Color_table)
3. [BMP Official Windows Documentation](https://learn.microsoft.com/en-us/dotnet/desktop/winforms/advanced/types-of-bitmaps?view=netframeworkdesktop-4.8)
4. [SDL Wiki - for debugging](https://wiki.libsdl.org/SDL2/SDL_PixelFormatEnum)
5. [JPEG Official Documentation](https://jpeg.org/jpeg/)
6. [JPEG Wiki](https://en.wikipedia.org/wiki/JPEG)
7. [C*ompressed Image File Formats JPEG, PNG, GIF, XBM, BMP. Your guideline to graphics files on the Web*, John Miano](https://books.ms/main/2C135A877CA819E55510F912CB6F9C98), Addison Wesley, 1999