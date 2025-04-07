#include <iostream>
#include <chrono>
#include <memory>
#include <string_view>
#include "Chip8.h"
#include "SuperChip.h"
#include <SDL2/SDL.h>
#include <map>
#include <filesystem>
#include <stdexcept>

struct EmulatorConfig {
    std::string romPath;
    Mode chipType = Mode::CHIP8;
    int scale = 15;
    bool enableDisassembler = false;
};

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [options] <rom_path>\n"
              << "Options:\n"
              << "  --chip <type>    Chip type (chip8 or superchip) [default: chip8]\n"
              << "  --scale <n>      Display scale factor [default: 15]\n"
              << "  --disasm         Enable instruction disassembly output [default: false]\n"
              << "  --help           Show this help message\n";
}

EmulatorConfig parseCommandLine(int argc, char* argv[]) {
    EmulatorConfig config;
    
    if (argc < 2) {
        printUsage(argv[0]);
        throw std::runtime_error("ROM path is required");
    }

    for (int i = 1; i < argc; ++i) {
        std::string_view arg(argv[i]);
        
        if (arg == "--help") {
            printUsage(argv[0]);
            std::exit(0);
        } else if (arg == "--chip" && i + 1 < argc) {
            std::string_view chipType(argv[++i]);
            if (chipType == "superchip") {
                config.chipType = Mode::SUPERCHIP;
            } else if (chipType == "chip8") {
                config.chipType = Mode::CHIP8;
            } else {
                throw std::runtime_error("Invalid chip type. Use 'chip8' or 'superchip'");
            }
        } else if (arg == "--scale" && i + 1 < argc) {
            config.scale = std::stoi(argv[++i]);
            if (config.scale < 1) {
                throw std::runtime_error("Scale must be positive");
            }
        } else if (arg == "--disasm") {
            config.enableDisassembler = true;
        } else if (config.romPath.empty()) {
            config.romPath = arg;
        } else {
            throw std::runtime_error("Unexpected argument: " + std::string(arg));
        }
    }

    if (!std::filesystem::exists(config.romPath)) {
        throw std::runtime_error("ROM file not found: " + config.romPath);
    }

    return config;
}

class SDLContext {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
public:
    SDLContext(const char* title, int width, int height, int flags) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error(std::string("SDL Init Error: ") + SDL_GetError());
        }
        
        window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                width, height, flags);
        if (!window) {
            SDL_Quit();
            throw std::runtime_error(std::string("Window creation error: ") + SDL_GetError());
        }
        
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error(std::string("Renderer creation error: ") + SDL_GetError());
        }
    }
    
    ~SDLContext() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    SDL_Renderer* getRenderer() const { return renderer; }
    SDL_Window* getWindow() const { return window; }
    
    // Prevent copying
    SDLContext(const SDLContext&) = delete;
    SDLContext& operator=(const SDLContext&) = delete;
};

const std::map<SDL_Scancode, int> KEYMAP = {
    { SDL_SCANCODE_1, 0x1 }, { SDL_SCANCODE_2, 0x2 }, { SDL_SCANCODE_3, 0x3 }, { SDL_SCANCODE_4, 0xC },
    { SDL_SCANCODE_Q, 0x4 }, { SDL_SCANCODE_W, 0x5 }, { SDL_SCANCODE_E, 0x6 }, { SDL_SCANCODE_R, 0xD },
    { SDL_SCANCODE_A, 0x7 }, { SDL_SCANCODE_S, 0x8 }, { SDL_SCANCODE_D, 0x9 }, { SDL_SCANCODE_F, 0xE },
    { SDL_SCANCODE_Z, 0xA }, { SDL_SCANCODE_X, 0x0 }, { SDL_SCANCODE_C, 0xB }, { SDL_SCANCODE_V, 0xF }
};

int main(int argc, char* argv[]) {
    try {
        EmulatorConfig config = parseCommandLine(argc, argv);
        
        // Create appropriate chip type
        std::unique_ptr<Chip8> chip8;
        if (config.chipType == Mode::SUPERCHIP) {
            chip8 = std::make_unique<SuperChip>();
        } else {
            chip8 = std::make_unique<Chip8>();
        }
        
        chip8->setMode(config.chipType);
        chip8->loadROM(config.romPath);
        
        // Initialize SDL with RAII
        SDLContext sdl("CHIP-8 Emulator", 
                      chip8->display.getWidth() * config.scale,
                      chip8->display.getHeight() * config.scale,
                      SDL_WINDOW_RESIZABLE);
        
        // Pre-allocate pixels array
        std::vector<SDL_Rect> pixels(chip8->display.getWidth() * chip8->display.getHeight());
        
        // Setup timing
        using Clock = std::chrono::high_resolution_clock;
        using Duration = std::chrono::duration<double>;
        
        const Duration frameTime(1.0/60.0);  // 60 Hz
        const Duration cpuCycleTime(1.0/500.0);  // 500 Hz
        
        auto lastFrameTime = Clock::now();
        auto lastCpuTime = Clock::now();
        
        bool running = true;
        SDL_Event event;
        
        while (running) {
            // Handle events
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
                else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    auto it = KEYMAP.find(event.key.keysym.scancode);
                    if (it != KEYMAP.end()) {
                        chip8->keypad[it->second] = (event.type == SDL_KEYDOWN);
                    }
                }
            }
            
            // Run CPU cycles
            auto now = Clock::now();
            while (now - lastCpuTime >= cpuCycleTime) {
                if (config.enableDisassembler) {
                    uint16_t opcode = chip8->fetch();
                    auto instruction = chip8->decode(opcode);
                    chip8->disassemble(instruction);
                    chip8->execute(instruction);
                } else {
                    chip8->emulateCycle();
                }
                lastCpuTime += std::chrono::duration_cast<std::chrono::steady_clock::duration>(cpuCycleTime);
            }
            
            // Update display if needed
            auto currentTime = Clock::now();
            Duration elapsed = currentTime - lastFrameTime;
            
            if (elapsed >= frameTime) {
                chip8->updateTimers();
                lastFrameTime = currentTime;
                
                // Clear renderer
                SDL_SetRenderDrawColor(sdl.getRenderer(), 0, 0, 0, 255);
                SDL_RenderClear(sdl.getRenderer());
                
                // Get window size for centering
                int winWidth, winHeight;
                SDL_GetWindowSize(sdl.getWindow(), &winWidth, &winHeight);
                
                // Update pixels
                int pixelCount = 0;
                for (int x = 0; x < chip8->display.getWidth(); ++x) {
                    for (int y = 0; y < chip8->display.getHeight(); ++y) {
                        if (chip8->display.display[y * chip8->display.getWidth() + x]) {
                            int xOffset = (winWidth - chip8->display.getWidth() * config.scale) / 2;
                            int yOffset = (winHeight - chip8->display.getHeight() * config.scale) / 2;
                            pixels[pixelCount++] = {
                                xOffset + x * config.scale,
                                yOffset + y * config.scale,
                                config.scale,
                                config.scale
                            };
                        }
                    }
                }
                
                // Draw pixels
                if (pixelCount > 0) {
                    SDL_SetRenderDrawColor(sdl.getRenderer(), 255, 255, 255, 255);
                    SDL_RenderFillRects(sdl.getRenderer(), pixels.data(), pixelCount);
                }
                
                SDL_RenderPresent(sdl.getRenderer());
            }
        }
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
