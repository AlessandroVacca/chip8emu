//
// Created by Alessandro Vacca on 06/04/25.
//

#include "SuperChip.h"

SuperChip::SuperChip() {
    setMode(Mode::SUPERCHIP);
}

void SuperChip::enableHiRes() {
    hiRes = true;
    display = Display(128, 64); // Set display to high resolution
}

void SuperChip::disableHiRes() {
    hiRes = false;
    display = Display(64, 32); // Set display to low resolution
}

void SuperChip::execute(Instruction i) {
    switch (i.opcode) {
        case 0x00:
            if (i.x == 0) {
                switch (i.nn) {
                    case 0xFB:
                        // Scroll right by 4px for each row
                            for (int y = 0; y < display.getHeight(); y++) {
                                int rowStart = y * display.getWidth();
                                std::move_backward(display.display.begin() + rowStart,
                                                   display.display.begin() + rowStart + display.getWidth() - 4,
                                                   display.display.begin() + rowStart + display.getWidth());
                                std::fill_n(display.display.begin() + rowStart, 4, false);
                            }
                    return;
                    case 0xFC:
                        // Scroll left by 4px for each row
                        for (int y = 0; y < display.getHeight(); y++) {
                            int rowStart = y * display.getWidth();
                            std::move(display.display.begin() + rowStart + 4,
                                      display.display.begin() + rowStart + display.getWidth(),
                                      display.display.begin() + rowStart);
                            std::fill_n(display.display.begin() + rowStart + display.getWidth() - 4, 4, false);
                        }
                    return;
                    case 0x0FD:
                        std::cout << "0x00FD, Exiting..." << std::endl;
                        exit(0);
                    case 0xFE:
                        // Set display mode to low resolution (always switch, regardless of current state)
                        disableHiRes();
                        return;
                    case 0xFF:
                        // Set display mode to high resolution
                        enableHiRes();
                        return;
                    default:
                        if (i.y == 0xC) {
                            // Scroll down by N pixels
                            std::move_backward(
                                display.display.begin(),
                                display.display.end() - display.getWidth() * i.n,
                                display.display.end()
                            );

                            // Clear the top i.n rows
                            std::fill_n(display.display.begin(), display.getWidth() * i.n, false);
                            return;
                        }
                        break;
                }
            }
            break;
        case 0x0D:
            if (i.n == 0) {
                // Draw 16x16 sprite at (Vx, Vy)
                uint8_t x = V[i.x] % display.getWidth();
                uint8_t y = V[i.y] % display.getHeight();
                for (int row = 0; row < 16; ++row) {
                    uint8_t sprite = memory[index + row];
                    for (int col = 0; col < 16; ++col) {
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
                    }
                }
                return;
            }
            break;
        case 0x0F:
            switch (i.nn) {
                case 0x30:
                    // Point I to 10-byte font sprite for digit VX (0..9)
                    index = V[i.x] * 10;
                    return;
                case 0x75:
                    // Read V0..VX from RPL user flags (X <= 7)
                    for (int j = 0; j < i.x && j <= 7; j++) {
                        V[j] = RPL[j];
                    }
                    return;
                case 0x85:
                    // Store V0..VX in RPL user flags (X <= 7)
                    for (int j = 0; j <= i.x && j <= 7; j++) {
                        RPL[j] = V[j];
                    }
                    return;
            }
            break;
    }
    Chip8::execute(i); // Call the base class execute method
}
