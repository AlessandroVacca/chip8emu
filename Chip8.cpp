#include "Chip8.h"
#include <fstream>
#include <sstream>
#include <iomanip>

Chip8::Chip8(): display(64, 32) {
    pc = 0x200; // Program Counter starts at 0x200
    index = 0; // Index Register
    delay_timer = 0; // Delay Timer
    sound_timer = 0; // Sound Timer

    // Initialize memory
    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }

    // Initialize registers
    for (int i = 0; i < 16; i++) {
        V[i] = 0;
    }
    // Initialize stack
    for (int i = 0; i < 16; i++) {
        stack.data[i] = 0;
    }

    // Initialize keypad
    for (int i = 0; i < 16; i++) {
        keypad[i] = false;
    }
    // Load fontset into memory
    uint8_t fontset[80] = {
        // Fontset data (5x5 pixels for each character)
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80 // F
    };
    for (int i = 0; i < 80; i++) {
        memory[0x050 + i] = fontset[i];
    }
}

void Chip8::clearDisplay() {
    std::fill(display.display.begin(), display.display.end(), false);
}

uint16_t Chip8::fetch() {
    // Fetch instruction from memory
    uint16_t instruction = (memory[pc] << 8) | memory[pc + 1];
    pc += 2; // Move to the next instruction
    return instruction;
}

Instruction Chip8::decode(uint16_t instruction) {
    // Decode instruction
    uint8_t opcode = (instruction & 0xF000) >> 12;
    uint8_t x = (instruction & 0x0F00) >> 8;
    uint8_t y = (instruction & 0x00F0) >> 4;
    uint8_t n = instruction & 0x000F;
    uint16_t nn = instruction & 0x00FF;
    uint16_t nnn = instruction & 0x0FFF;
    return {opcode, x, y, n, nn, nnn};
}

void Chip8::execute(Instruction i) {
    // Execute instruction
    switch (i.opcode) {
        case 0x00:
            // Clear the display
            switch (i.nnn) {
                case 0x0E0:
                    clearDisplay();
                    break;
                case 0x0EE:
                    // Return from subroutine
                    pc = stack.pop();
                    break;
                default:
                    if (i.nnn == 0) {
                        break;
                    }
                    // Call RCA 1802 program at address nnn
                    //throw std::runtime_error("0NNN instruction: RCA 1802 program at address " + std::to_string(i.nnn));
                    std::cout << "0NNN instruction: RCA 1802 program: 0x" << std::hex << (i.opcode << 12 | i.nnn) << std::endl;
                    break;
            }
            break;
        case 0x01:
            // Jump to address nnn
            pc = i.nnn;
            break;
        case 0x02:
            // Call subroutine at nnn
            stack.push(pc);
            pc = i.nnn;
            break;
        case 0x03:
            // Skip next instruction if Vx == nn
            if (V[i.x] == i.nn) {
                pc += 2;
            }
            break;
        case 0x04:
            // Skip next instruction if Vx != nn
            if (V[i.x] != i.nn) {
                pc += 2;
            }
            break;
        case 0x05:
            // Skip next instruction if Vx == Vy
            if (V[i.x] == V[i.y]) {
                pc += 2;
            }
            break;
        case 0x09:
            // Skip next instruction if Vx != Vy
            if (V[i.x] != V[i.y]) {
                pc += 2;
            }
            break;
        case 0x06:
            // Set Vx to nn
            V[i.x] = i.nn;
            break;
        case 0x07:
            // Add nn to Vx
            V[i.x] += i.nn;
            break;
        case 0x0A:
            // Set I to nnn
            index = i.nnn;
            break;

        case 0x08: // Math operations
            switch (i.n) {
                case 0x00:
                    // Set Vx to Vy
                    V[i.x] = V[i.y];
                    break;
                case 0x01:
                    // Set Vx to Vx OR Vy
                    V[i.x] |= V[i.y];
                    if (!super_chip) {
                        V[0xF] = 0; // Clear carry flag
                    }
                    break;
                case 0x02:
                    // Set Vx to Vx AND Vy
                    V[i.x] &= V[i.y];
                    if (!super_chip) {
                        V[0xF] = 0; // Clear carry flag
                    }
                    break;
                case 0x03:
                    // Set Vx to Vx XOR Vy
                    V[i.x] ^= V[i.y];
                    if (!super_chip) {
                        V[0xF] = 0; // Clear carry flag
                    }
                    break;
                case 0x04: {
                    // Add Vy to Vx, set VF to 1 if there is a carry
                    const uint8_t x = V[i.x];
                    V[i.x] += V[i.y];
                    V[0xF] = (x + V[i.y]) > 0xFF ? 1 : 0;
                    break;
                }
                case 0x05: {
                    // Subtract Vy from Vx, set VF to 0 if there is a borrow
                    const uint8_t x = V[i.x];
                    V[i.x] = V[i.x] - V[i.y];
                    V[0xF] = (x >= V[i.y]) ? 1 : 0;
                    break;
                }
                case 0x07: {
                    // Set Vx to Vy - Vx, set VF to 0 if there is a borrow
                    const uint8_t x = V[i.x];
                    V[i.x] = V[i.y] - V[i.x];
                    V[0xF] = (V[i.y] >= x) ? 1 : 0;
                    break;
                }
                case 0x06: {
                    if (!super_chip) {
                        // Move Vx to Vy
                        V[i.x] = V[i.y];
                    }
                    // Shift Vx right by 1, set VF to the least significant bit of Vx before the shift
                    const uint8_t x = V[i.x];
                    V[i.x] >>= 1;
                    V[0xF] = x & 0x01;
                    break;
                }
                case 0x0E: {
                    if (!super_chip) {
                        // Move Vx to Vy
                        V[i.x] = V[i.y];
                    }
                    // Shift Vx left by 1, set VF to the most significant bit of Vx before the shift
                    const uint8_t x = V[i.x];
                    V[i.x] <<= 1;
                    V[0xF] = (x & 0x80) >> 7;
                    break;
                }
            }
            break;
        case 0x0B:
            if (super_chip) {
                // Jump to address xnn + VX
                pc = i.nnn + V[i.x];
            } else {
                // Jump to address nnn + V0
                pc = i.nnn + V[0];
            }
            break;
        case 0x0C:
            // Generate random number and AND with nn, save in Vx
            V[i.x] = rand() % 256 && i.nn;
            break;
        case 0x0D: {
            uint8_t x = V[i.x] % display.getWidth();
            uint8_t y = V[i.y] % display.getHeight();
            V[0xF] = 0;

            for (int row = 0; row < i.n; ++row) {
                uint8_t sprite = memory[index + row];
                for (int col = 0; col < 8; ++col) {
                    // Cast sprite to bool, using bitmasking
                    bool pixel = (sprite & (0x80 >> col)) != 0;
                    if (pixel) {
                        if (display.display[display.getWidth() * (y + row) + x + col]) {
                            V[0xF] = 1; // Collision detected
                            display.display[display.getWidth() * (y + row) + x + col] = false; // Erase pixel
                        } else {
                            display.display[display.getWidth() * (y + row) + x + col] = true;
                        }
                    }
                    if (x + col >= display.getWidth()) {
                        break; // Prevent overflow
                    }
                }
                if (y + row >= display.getHeight()) {
                    break; // Prevent overflow
                }
            }
            break;
        }
        case 0x0E:
            switch (i.nn) {
                case 0x9E:
                    // Skip next instruction if key with value of Vx is pressed
                    if (keypad[V[i.x]]) {
                        pc += 2;
                    }
                    break;
                case 0xA1:
                    // Skip next instruction if key with value of Vx is not pressed
                    if (!keypad[V[i.x]]) {
                        pc += 2;
                    }
                    break;
            }
            break;
        case 0x0F:
            switch (i.nn) {
                case 0x07:
                    // Set Vx to the value of the delay timer
                    V[i.x] = delay_timer;
                    break;
                case 0x15:
                    // Set the delay timer to Vx
                    delay_timer = V[i.x];
                    break;
                case 0x18:
                    // Set the sound timer to Vx
                    sound_timer = V[i.x];
                    break;
                case 0x1E:
                    // Add Vx to I
                    index += V[i.x];
                    break;
                case 0x0A: {
                    // Wait for a key press and release
                    static int lastPressed = -1;

                    // If we haven't detected a pressed key yet
                    if (lastPressed == -1) {
                        for (int j = 0; j < 16; j++) {
                            if (keypad[j]) {
                                lastPressed = j;
                                break;
                            }
                        }
                        // Keep waiting for a key press
                        pc -= 2;
                    }
                    // If we have a pressed key, wait for release
                    else if (!keypad[lastPressed]) {
                        V[i.x] = lastPressed;
                        lastPressed = -1; // Reset for next time
                    }
                    // Key still pressed, keep waiting
                    else {
                        pc -= 2;
                    }
                    break;
                }
                case 0x29:
                    // Set I to the location of the sprite for the character in Vx
                    index = V[i.x] * 5; // Each character is 5 bytes
                    break;
                case 0x33:
                    // Store BCD representation of Vx in memory at I, I+1, I+2
                    memory[index] = V[i.x] / 100;
                    memory[index + 1] = (V[i.x] / 10) % 10;
                    memory[index + 2] = V[i.x] % 10;
                    break;
                case 0x55:
                    if (super_chip) {
                        // Store registers V0 to Vx in memory starting at I
                        for (int j = 0; j <= i.x; j++) {
                            memory[index + j] = V[j];
                        }
                    } else {
                        // Store registers V0 to Vx in memory starting at I
                        for (int j = 0; j <= i.x; j++) {
                            memory[index] = V[j];
                            index++;
                        }
                    }
                    break;
                case 0x65:
                    if (super_chip) {
                        // Read registers V0 to Vx from memory starting at I
                        for (int j = 0; j <= i.x; j++) {
                            V[j] = memory[index + j];
                        }
                    } else {
                        // Read registers V0 to Vx from memory starting at I
                        for (int j = 0; j <= i.x; j++) {
                            V[j] = memory[index];
                            index++;
                        }
                    }
                    break;
            }
            break;
        default:
            // Handle other opcodes here
            std::cout << "Unknown instruction: 0x" << std::hex << (i.opcode << 12 | i.nnn) << std::endl;
            break;
    }
}

void Chip8::loadROM(const std::string &path) {
    std::ifstream rom(path, std::ios::binary | std::ios::ate);
    if (!rom.is_open()) {
        throw std::runtime_error("Unable to open ROM file: " + path);
    }

    std::streamsize size = rom.tellg();
    rom.seekg(0, std::ios::beg);

    if (size > 3584) {
        throw std::runtime_error("ROM size exceeds memory capacity");
    }

    rom.read(reinterpret_cast<char *>(&memory[0x200]), size);
    rom.close();
}

void Chip8::emulateCycle() {
    // Fetch, decode, and execute instruction
    uint16_t instruction = fetch();
    Instruction decodedInstruction = decode(instruction);
    execute(decodedInstruction);
}

void Chip8::printDisplay() {
    for (int y = 0; y < display.getHeight(); y++) {
        for (int x = 0; x < display.getWidth(); x++) {
            std::cout << (display.display[y * display.getWidth() + x] ? "█" : " ");
        }
        std::cout << std::endl;
    }
}

void Chip8::setMode(Mode mode) {
    super_chip = (mode == Mode::SUPERCHIP);
}

void Chip8::updateTimers() {
    // Update timers
    if (delay_timer > 0) {
        delay_timer--;
    }
    if (sound_timer > 0) {
        sound_timer--;
        if (sound_timer == 0) {
            // Beep sound (not implemented)
            std::cout << "Beep!" << std::endl;
        }
    }
}

std::string Chip8::disassemble(Instruction i) {
    // convert decoded instruction to hex string
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(4)
       << ((i.opcode << 12) | (i.x << 8) | (i.y << 4) | i.n);
    std::string instructionString = ss.str();
    std::string result;

    // disassemble instruction
    if (instructionString == "00E0") {
        result = "clear";
    } else if (instructionString == "00EE") {
        result = "return";
    } else if (instructionString == "00FB") {
        result = "scroll-right";
    } else if (instructionString == "00FC") {
        result = "scroll-left";
    } else if (instructionString.substr(0, instructionString.size() - 1) == "00C") {
        result = "scroll-down " + std::to_string(i.n);
    }
    else if (instructionString == "00FD") {
        result = "exit";
    } else if (instructionString == "00FE") {
        result = "lores";
    } else if (instructionString == "00FF") {
        result = "hires";
    }
    else if (instructionString[0] == '1') {
        result = "jump 0x" + instructionString.substr(1);
    } else if (instructionString[0] == '2') {
        result = "call 0x" + instructionString.substr(1);
    } else if (instructionString[0] == '3') {
        result = "skip if V(0x" + std::string(1, instructionString[1]) + ") == 0x" + instructionString.substr(2);
    } else if (instructionString[0] == '4') {
        result = "skip if V(0x" + std::string(1, instructionString[1]) + ") != 0x" + instructionString.substr(2);
    } else if (instructionString[0] == '5') {
        result = "skip if V(0x" + std::string(1, instructionString[1]) + ") == V(0x" + instructionString[2] + ")";
    } else if (instructionString[0] == '6') {
        result = "V(0x" + std::string(1, instructionString[1]) + ") := 0x" + instructionString.substr(2);
    } else if (instructionString[0] == '7') {
        result = "V(0x" + std::string(1, instructionString[1]) + ") += 0x" + instructionString.substr(2);
    } else if (instructionString[0] == '8') {
        if (instructionString[3] == '0') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[2] + ")";
        } else if (instructionString[3] == '1') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[1] + ") OR V(0x" + instructionString[2] + ")";
        } else if (instructionString[3] == '2') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[1] + ") AND V(0x" + instructionString[2] + ")";
        } else if (instructionString[3] == '3') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[1] + ") XOR V(0x" + instructionString[2] + ")";
        } else if (instructionString[3] == '4') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[1] + ") + V(0x" + instructionString[2] + ")";
        } else if (instructionString[3] == '5') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[1] + ") - V(0x" + instructionString[2] + ")";
        } else if (instructionString[3] == '6') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[1] + ") >> 1";
        } else if (instructionString[3] == '7') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[2] + ") - V(0x" + instructionString[1] + ")";
        } else if (instructionString[3] == 'E') {
            result = "V(0x" + std::string(1, instructionString[1]) + ") := V(0x" + instructionString[1] + ") << 1";
        }
    } else if (instructionString[0] == '9') {
        result = "skip if V(0x" + std::string(1, instructionString[1]) + ") != V(0x" + instructionString[2] + ")";
    } else if (instructionString[0] == 'A') {
        result = "I := 0x" + instructionString.substr(1);
    } else if (instructionString[0] == 'B') {
        result = "jump V0 + 0x" + instructionString.substr(1);
    } else if (instructionString[0] == 'C') {
        result = "rand, bitmask V(0x" + std::string(1, instructionString[1]) + ")";
    } else if (instructionString[0] == 'D') {
        result = "draw (" + std::to_string(i.x) + ", " + std::to_string(i.y) +
                             "), height " + std::to_string(i.n);
    } else if (instructionString[0] == 'E') {
        if (instructionString.substr(2) == "9E") {
            result = "skip if key V(0x" + std::string(1, instructionString[1]) + ") pressed";
        } else if (instructionString.substr(2) == "A1") {
            result = "skip if key V(0x" + std::string(1, instructionString[1]) + ") not pressed";
        }
    } else if (instructionString[0] == 'F') {
        if (instructionString.substr(2) == "07") {
            result = "delay store V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "0A") {
            result = "wait for key V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "15") {
            result = "delay set V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "18") {
            result = "sound set V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "1E") {
            result = "I += V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "29") {
            result = "I := addr sprite V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "33") {
            result = "BCD store V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "55") {
            result = "store V0 to V(0x" + std::string(1, instructionString[1]) + ")";
        } else if (instructionString.substr(2) == "65") {
            result = "load V0 to V(0x" + std::string(1, instructionString[1]) + ")";
        }
    }

    return result;
}
