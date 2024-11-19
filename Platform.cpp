#include "Platform.hpp"
#include <SDL2/SDL.h>
#include <cstring>
#include<iostream>
#include <stdexcept>

Platform::Platform(const char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
{
    // Initialize SDL video subsystem
    std::cout<< "Initializing SDL..."<<std::endl;
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        throw std::runtime_error("Failed to initialize SDL");
    }
    std::cout<< "SDL initialized."<<std::endl;

    std::cout<< "Creating window..."<<std::endl;
    // Create SDL window
    window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        windowWidth, windowHeight,
        SDL_WINDOW_RESIZABLE
    );

    if (!window)
    {
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL Window");
    }
    std::cout << "Window created successfully.\n";
    // Create SDL renderer
    std::cout << "Creating renderer...\n";
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL Renderer");
    }
    std::cout << "Renderer created successfully.\n";

    std::cout << "Creating texture...\n";
    // Create SDL texture for rendering
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        textureWidth,
        textureHeight
    );

    if (!texture)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Failed to create SDL Texture");
    }
    std::cout << "Texture created successfully.\n";
}

Platform::~Platform()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Platform::Update(const void* buffer, int pitch)
{
    if (!buffer) {
        // For testing: fill the screen with a solid color if buffer is null
        uint32_t testColor = 0xFFFF00FF; // 
        uint32_t testBuffer[640 * 320];  // Adjust size to match your texture
        std::fill(std::begin(testBuffer), std::end(testBuffer), testColor);
        buffer = testBuffer;
        pitch = 640 * sizeof(uint32_t); // Adjust for texture width in bytes
    }
    // Update the SDL texture with new pixel data
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);

    // Clear the renderer and copy the texture to the rendering target
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

    // Present the updated frame
    SDL_RenderPresent(renderer);
}

bool Platform::ProcessInput(uint8_t* keys)
{
    bool quit = false;
    SDL_Event event;

    // Poll for events
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                {
                    quit = true;
                }break;

            case SDL_KEYDOWN:
            std::cout << "Key pressed: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    {   quit = true;
                    }break;
                    case SDLK_x: keys[0] = 1; break;
                    case SDLK_1: keys[1] = 1; break;
                    case SDLK_2: keys[2] = 1; break;
                    case SDLK_3: keys[3] = 1; break;
                    case SDLK_q: keys[4] = 1; break;
                    case SDLK_w: keys[5] = 1; break;
                    case SDLK_e: keys[6] = 1; break;
                    case SDLK_a: keys[7] = 1; break;
                    case SDLK_s: keys[8] = 1; break;
                    case SDLK_d: keys[9] = 1; break;
                    case SDLK_z: keys[0xA] = 1; break;
                    case SDLK_c: keys[0xB] = 1; break;
                    case SDLK_4: keys[0xC] = 1; break;
                    case SDLK_r: keys[0xD] = 1; break;
                    case SDLK_f: keys[0xE] = 1; break;
                    case SDLK_v: keys[0xF] = 1; break;
                }
                break;

            case SDL_KEYUP:
            std::cout << "Key released: " << SDL_GetKeyName(event.key.keysym.sym) << std::endl;
                switch (event.key.keysym.sym)
                {
                    case SDLK_x: keys[0] = 0; break;
                    case SDLK_1: keys[1] = 0; break;
                    case SDLK_2: keys[2] = 0; break;
                    case SDLK_3: keys[3] = 0; break;
                    case SDLK_q: keys[4] = 0; break;
                    case SDLK_w: keys[5] = 0; break;
                    case SDLK_e: keys[6] = 0; break;
                    case SDLK_a: keys[7] = 0; break;
                    case SDLK_s: keys[8] = 0; break;
                    case SDLK_d: keys[9] = 0; break;
                    case SDLK_z: keys[0xA] = 0; break;
                    case SDLK_c: keys[0xB] = 0; break;
                    case SDLK_4: keys[0xC] = 0; break;
                    case SDLK_r: keys[0xD] = 0; break;
                    case SDLK_f: keys[0xE] = 0; break;
                    case SDLK_v: keys[0xF] = 0; break;
                }
                break;
        }
    }

    return quit;
}
