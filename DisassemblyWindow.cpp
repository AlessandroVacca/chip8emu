#include "DisassemblyWindow.h"
#include <stdexcept>

DisassemblyWindow::DisassemblyWindow(const char* title, int x, int y, int width, int height) 
    : window(nullptr), renderer(nullptr), font(nullptr) {
    
    // Initialize TTF if not already initialized
    if (!TTF_WasInit() && TTF_Init() < 0) {
        throw std::runtime_error(std::string("TTF Init Error: ") + TTF_GetError());
    }

    // Create window
    window = SDL_CreateWindow(
        title,
        x, y, width, height,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        cleanup();
        throw std::runtime_error(std::string("Window creation error: ") + SDL_GetError());
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cleanup();
        throw std::runtime_error(std::string("Renderer creation error: ") + SDL_GetError());
    }

    // Load font (using a system font path - modify for your system)
    #ifdef _WIN32
    const char* fontPath = "C:\\Windows\\Fonts\\consola.ttf";
    #elif __APPLE__
    const char* fontPath = "/System/Library/Fonts/Monaco.ttf";
    #else  // Linux
    const char* fontPath = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    #endif

    font = TTF_OpenFont(fontPath, 14);
    if (!font) {
        cleanup();
        throw std::runtime_error(std::string("Font loading error: ") + TTF_GetError());
    }
}

DisassemblyWindow::~DisassemblyWindow() {
    cleanup();
}

void DisassemblyWindow::cleanup() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
}

void DisassemblyWindow::addInstruction(const std::string& instruction) {
    // Add instruction with its number to the circular buffer
    instructions.push_back({++instructionCount, instruction});
    if (instructions.size() > MAX_LINES) {
        instructions.pop_front();
    }
}

void DisassemblyWindow::render() {
    if (!window || !renderer || !font) return;

    // Clear window
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render each instruction
    int y = PADDING;
    for (const auto& [number, instruction] : instructions) {
        // Format with actual instruction number
        std::string lineText = std::to_string(number) + ": " + instruction;
        renderText(lineText, y);
        y += LINE_HEIGHT;
    }

    SDL_RenderPresent(renderer);
}

void DisassemblyWindow::renderText(const std::string& text, int y) {
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    // Use blended rendering for smooth, high-quality text
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), textColor);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect dstRect = {
                PADDING,
                y,
                surface->w,
                surface->h
            };
            SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}
