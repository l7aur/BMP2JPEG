#pragma once

#include <SDL2/SDL.h>

namespace  Util {
    class ImageRenderer
    {
    public:
        ImageRenderer() = default;
        ~ImageRenderer();
        void runGameLoop() const;
        int init();
        int initTexture(int imageWidth, int imageHeight, const uint32_t* pixels);

    private:
        const int WINDOW_WIDTH{512};
        const int IMAGE_HEIGHT{512};
        int textureWidth{0};
        int textureHeight{0};
        SDL_Window *window{nullptr};
        SDL_Renderer *renderer{nullptr};
        SDL_Texture *texture{nullptr};

        int initRenderer();
        int initWindow();

        static int initContext();
    };
}
