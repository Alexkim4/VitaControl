#ifndef EIGHTBITDO_CONTROLLER_H
#define EIGHTBITDO_CONTROLLER_H

#include "../controller.h"

struct EightBitDoDInputReport
{
    uint8_t leftX;
    uint8_t leftY;
    uint8_t rightX;
    uint8_t rightY;

    uint8_t dpad : 4;  // Hat Switch (0~7: 방향, 8/15: 중립)
    uint8_t      : 4;

    uint8_t a  : 1;    // Button 1 (A)
    uint8_t b  : 1;    // Button 2 (B)
    uint8_t    : 1;
    uint8_t x  : 1;    // Button 4 (X)
    uint8_t y  : 1;    // Button 5 (Y)
    uint8_t    : 1;
    uint8_t l1 : 1;    // Button 7 (L1)
    uint8_t r1 : 1;    // Button 8 (R1)

    uint8_t l2     : 1;// Button 9 (L2)
    uint8_t r2     : 1;// Button 10 (R2)
    uint8_t select : 1;// Button 11 (Select)
    uint8_t start  : 1;// Button 12 (Start)
    uint8_t home   : 1;// Button 13 (PS 버튼)
    uint8_t stickL : 1;// Button 14 (L3)
    uint8_t stickR : 1;// Button 15 (R3)
    uint8_t        : 1;
}
__attribute__((packed));

class EightBitDoController: public Controller
{
    public:
        EightBitDoController(uint32_t mac0, uint32_t mac1, int port);
        void processReport(uint8_t *buffer, size_t length);
};

#endif // EIGHTBITDO_CONTROLLER_H
