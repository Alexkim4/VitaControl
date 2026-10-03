#ifndef EIGHTBITDO_CONTROLLER_H
#define EIGHTBITDO_CONTROLLER_H

#include "../controller.h"

// 8BitDo Bluetooth HID Report Layout (Pro 2, Pro 3, SN30 Pro in D-Input / Bluetooth mode)
struct EightBitDoReport0x01
{
    uint8_t reportId; // 0x01
    uint8_t dpad;     // Hat Switch (0=Up, 1=NE, 2=Right, 3=SE, 4=Down, 5=SW, 6=Left, 7=NW, 8=Neutral)
    uint8_t leftX;    // 0..255 (Center ~128)
    uint8_t leftY;    // 0..255 (Center ~128)
    uint8_t rightX;   // 0..255 (Center ~128)
    uint8_t rightY;   // 0..255 (Center ~128)
    uint8_t triggerR; // 0..255 (Analog RT)
    uint8_t triggerL; // 0..255 (Analog LT)

    uint8_t cross    : 1; // South (B button)
    uint8_t circle   : 1; // East (A button)
    uint8_t paddleR  : 1; // PR
    uint8_t square   : 1; // West (Y button)
    uint8_t triangle : 1; // North (X button)
    uint8_t paddleL  : 1; // PL
    uint8_t l1       : 1; // L1 bumper
    uint8_t r1       : 1; // R1 bumper

    uint8_t          : 2;
    uint8_t select   : 1; // Select / Back / Minus
    uint8_t start    : 1; // Start / Plus
    uint8_t home     : 1; // Home / Guide / PS button
    uint8_t l3       : 1; // Left stick click
    uint8_t r3       : 1; // Right stick click
    uint8_t          : 1;
}
__attribute__((packed));

class EightBitDoController: public Controller
{
    public:
        EightBitDoController(uint32_t mac0, uint32_t mac1, int port);
        void processReport(uint8_t *buffer, size_t length);
};

#endif // EIGHTBITDO_CONTROLLER_H
