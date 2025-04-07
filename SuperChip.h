//
// Created by Alessandro Vacca on 06/04/25.
//

#ifndef SUPERCHIP_H
#define SUPERCHIP_H
#include "Chip8.h"


class SuperChip : public Chip8 {
    private:
      bool hiRes = false;
    uint8_t RPL[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    public:
      SuperChip();
      void enableHiRes();
      void disableHiRes();
      bool isHiRes() { return hiRes; }
      void execute(Instruction i) override;
};



#endif //SUPERCHIP_H
