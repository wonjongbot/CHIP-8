#include "Platform.hpp"
#include <SDL2/SDL.h>

// constructor
Platform::Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight){
    // initialize SDL library
    SDL_Init(SDL_INIT_VIDEO);

    // create a visible window at (0,0) on screen, with size of windowWidth*windowHeight
    window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);

    // create a renderer with the window created above
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // texture using pixelformat rgba8888
    texture = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight
    );
}

// deconstructor; safely deallocate texture, renderer, and window
Platform::~Platform(){
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::Update(void const* buffer, int pitch){
    // update texture from buffer
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    // clear render on screen
    SDL_RenderClear(renderer);
    // copy entire texture to destination
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    // render updated graphics
    SDL_RenderPresent(renderer);
}

bool Platform::ProcessInput(uint8_t* keys){
    bool quit = false;

}