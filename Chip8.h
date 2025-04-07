//
// Created by Alessandro Vacca on 06/04/25.
//

#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>
#include <iostream>

struct Instruction {
    uint8_t opcode; // The opcode of the instruction
    uint8_t x; // The first operand
    uint8_t y; // The second operand
    uint8_t n; // The third operand
    uint16_t nn; // The immediate value
    uint16_t nnn; // The address
};

enum class Mode {
    CHIP8,
    SUPERCHIP
};

struct Chip8Stack {
    uint16_t data[16]{};
    uint8_t sp = 0;

    void push(uint16_t value) {
        if (sp < 16) {
            data[sp++] = value;
        }
        // else: stack overflow (could add error handling)
    }

    uint16_t pop() {
        if (sp > 0) {
            return data[--sp];
        }
        // else: stack underflow (could add error handling)
        return 0;
    }
};

class Display {
    int width;
    int height;
public:
    std::vector<bool> display;
    Display(int width, int height) {
        this->width = width;
        this->height = height;
        display = std::vector<bool>(width * height, false);
    };
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};

class Chip8 {
    /*
     * Memory: CHIP-8 has direct access to up to 4 kilobytes of RAM
     * Display: 64 x 32 pixels (or 128 x 64 for SUPER-CHIP) monochrome, ie. black or white
     * A program counter, often called just “PC”, which points at the current instruction in memory
     * One 16-bit index register called “I” which is used to point at locations in memory
     * A stack for 16-bit addresses, which is used to call subroutines/functions and return from them
     * An 8-bit delay timer which is decremented at a rate of 60 Hz (60 times per second) until it reaches 0
     * An 8-bit sound timer which functions like the delay timer, but which also gives off a beeping sound as long as it’s not 0
     * 16 8-bit (one byte) general-purpose variable registers numbered 0 through F hexadecimal, ie. 0 through 15 in decimal, called V0 through VF
     * VF is also used as a flag register; many instructions will set it to either 1 or 0 based on some rule, for example using it as a carry flag
     */
    uint16_t pc; // Program Counter
    Chip8Stack stack; // Stack with push/pop
    uint8_t delay_timer; // Delay Timer
    uint8_t sound_timer; // Sound Timer

    void clearDisplay(); // Clear display

protected:
    bool super_chip = false; // Super Chip mode
    uint8_t V[16]{}; // Registers
    uint8_t memory[4096]{}; // Memory
    uint16_t index; // Index Register
public:
    Chip8(); // Constructor
    uint16_t fetch(); // Fetch instruction
    Instruction decode(uint16_t instruction); // Decode instruction
    virtual void execute(Instruction i); // Execute instruction
    void loadROM(const std::string &path); // Load ROM file
    void emulateCycle(); // Emulate a single cycle
    void printDisplay(); // Print display (for debugging)
    Display display; // Display
    bool keypad[16]{}; // Keypad
    void setMode(Mode mode);
    void disassemble(Instruction i); // Disassemble instruction (for debugging)
    void updateTimers(); // Update timers
};

#endif //CHIP8_H
