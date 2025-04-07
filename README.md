# CHIP-8/SuperCHIP Emulator

A modern C++20 emulator for CHIP-8 and SuperCHIP with high-resolution graphics support, configurable display scaling, and instruction disassembly capabilities.

## Features

- 🎮 Full CHIP-8 and SuperCHIP instruction set support
- 🖥️ Dynamic resolution switching (64x32 and 128x64)
- 🔍 Real-time instruction disassembler
- 📏 Configurable display scaling
- ⚡ Modern C++20 implementation
- 🎯 RAII-based resource management
- 🛠️ Modern CMake build system

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 10+, or MSVC 2019+)
- CMake 4.0 or higher
- SDL2 development libraries
- Git (for cloning the repository)

### Installing Dependencies

#### macOS
```bash
brew install cmake sdl2
```

#### Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev
```

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/chip8emu.git
cd chip8emu

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build .
```

## Usage

### Basic Usage
```bash
./chip8emu <rom_path>
```

### Command Line Options
```bash
Usage: chip8emu [options] <rom_path>
Options:
  --chip <type>    Chip type (chip8 or superchip) [default: chip8]
  --scale <n>      Display scale factor [default: 15]
  --disasm         Enable instruction disassembly output
  --help           Show this help message
```

### Examples
```bash
# Run a CHIP-8 ROM
./chip8emu games/tetris.ch8

# Run a SuperCHIP ROM with increased display size
./chip8emu --chip superchip --scale 20 games/snake.ch8

# Run with instruction disassembly output
./chip8emu --disasm games/pong.ch8

# Full debug mode
./chip8emu --chip superchip --scale 20 --disasm games/invaders.ch8
```

## Controls

The emulator uses a standard QWERTY keyboard mapping for the CHIP-8's hexadecimal keypad:

```
CHIP-8 Key   Keyboard
---------   ---------
1 2 3 C     1 2 3 4
4 5 6 D     Q W E R
7 8 9 E     A S D F
A 0 B F     Z X C V
```

## Technical Details

### Display Modes
- CHIP-8: 64x32 pixels monochrome display
- SuperCHIP: Supports both 64x32 (low resolution) and 128x64 (high resolution)

### Timing
- CPU frequency: 500Hz
- Display refresh rate: 60Hz
- Timers (delay and sound): 60Hz

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments
 
- [Tobiasvl's Guide to making a CHIP-8 emulator](https://tobiasvl.github.io/blog/write-a-chip-8-emulator/)
- [John Earnest's Mastering SuperChip](https://johnearnest.github.io/Octo/docs/SuperChip.html)
- [Cowgod's CHIP-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [SDL2 Documentation](https://wiki.libsdl.org/)
