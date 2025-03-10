#include "ImageRenderer.h"

#include <iostream>

ImageRenderer::~ImageRenderer()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void ImageRenderer::runGameLoop() const
{
    SDL_Rect displayRectangle = {0, 0, textureWidth, textureHeight};
    std::cout << textureWidth << ' ' << textureHeight << '\n';
    bool running = true;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
            running = (event.type != SDL_QUIT);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, &displayRectangle, &displayRectangle);
        SDL_RenderPresent(renderer);
    }
}

int ImageRenderer::init()
{
    if (initContext() < 0)
        return -1;
    if (initWindow() < 0)
        return -1;
    return initRenderer();
}

int ImageRenderer::initRenderer()
{
    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Unable to initialize the renderer!\n"
                  << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    return 0;
}

int ImageRenderer::initTexture(const int imageWidth, const int imageHeight, const uint32_t *pixels)
{
    if(pixels == nullptr) {
        std::cerr << "Unable to render anything, pixel pointer points to nothing!\n";
        return -1;
    }
    textureWidth = imageWidth;
    textureHeight = imageHeight;
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        imageWidth,
        imageHeight);

    return SDL_UpdateTexture(texture, nullptr, pixels, imageWidth * sizeof(uint32_t));
}

//     SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, HEIGHT);
//     Uint32 *pixels = new Uint32[WINDOW_WIDTH * IMAGE_HEIGHT];
//     for (int i = 0; i < IMAGE_HEIGHT; i++)
//         for (int j = 0; j < WINDOW_WIDTH; j++)
//         {
//             int index = i * WINDOW_WIDTH + j;
//             pixels[index] = 0xFF'00'00'00;
//         }

int ImageRenderer::initWindow()
{
    window = SDL_CreateWindow(
        "Image reconstruction",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        IMAGE_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Unable to initialize the window!\n";
        return -1;
    }
    return 0;
}

int ImageRenderer::initContext()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Unable to initialize SDL context!\n";
        return -1;
    }
    return 0;
}
