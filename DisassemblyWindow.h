#ifndef DISASSEMBLYWINDOW_H
#define DISASSEMBLYWINDOW_H

#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <deque>
#include <string>
#include <utility>
#include <cstdint>

class DisassemblyWindow {
public:
    DisassemblyWindow(const char* title, int x, int y, int width, int height);
    ~DisassemblyWindow();

    void addInstruction(const std::string& instruction);
    void render();
    bool isOpen() const { return window != nullptr && !wasClosed; }
    void checkEvent(const SDL_Event& event) {
        if (event.type == SDL_WINDOWEVENT && 
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(window)) {
            wasClosed = true;
        }
    }

private:
    bool wasClosed = false;
    static constexpr size_t MAX_LINES = 25;
    static constexpr int LINE_HEIGHT = 24;
    static constexpr int PADDING = 15;

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::deque<std::pair<uint32_t, std::string>> instructions;  // <instruction number, text>
    uint32_t instructionCount = 0;  // Running instruction counter

    void cleanup();
    void renderText(const std::string& text, int y);
};

#endif // DISASSEMBLYWINDOW_H
