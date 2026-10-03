#include <psp2kern/ctrl.h>
#include "eightbitdo_controller.h"

// 8BitDo Pro 3 (VID 0x2DC8, PID 0x6009) D-mode Bluetooth input report, verified from on-device logs:
//
//   [0]  0x04  report ID
//   [1]  D-pad hat (0=N, 1=NE, 2=E, 3=SE, 4=S, 5=SW, 6=W, 7=NW, 0x0F=neutral)
//   [2]  left stick X    [3] left stick Y   (0x80 center, 0x00 = left/up)
//   [4]  right stick X   [5] right stick Y
//   [6]  R2 analog       [7] L2 analog
//   [8]  0x01 East(○)  0x02 South(✕)  0x08 North(△)  0x10 West(□)  0x40 L1  0x80 R1
//   [9]  0x01 L2  0x02 R2  0x04 Select  0x08 Start  0x10 Home  0x20 L3  0x40 R3

#define EIGHTBITDO_REPORT_ID 0x04

static inline uint8_t filterDeadzone(uint8_t val)
{
    if (val >= 118 && val <= 138)
        return 128;
    return val;
}

EightBitDoController::EightBitDoController(uint32_t mac0, uint32_t mac1, int port): Controller(mac0, mac1, port)
{
    // Send an empty write request just to receive a response, which kicks off the read loop
    static uint8_t report[4] = {};
    requestReport(HID_REQUEST_WRITE, report, sizeof(report));
}

void EightBitDoController::processReport(uint8_t *buffer, size_t length)
{
    // Only process input reports; anything else keeps the previous state
    if (buffer[0] != EIGHTBITDO_REPORT_ID || length < 10)
        return;

    controlData.buttons = 0;

    uint8_t dpad = buffer[1] & 0x0F;
    uint8_t btn1 = buffer[8];
    uint8_t btn2 = buffer[9];

    // D-pad
    if (dpad == DPAD_NW || dpad == DPAD_N || dpad == DPAD_NE) controlData.buttons |= SCE_CTRL_UP;
    if (dpad == DPAD_NE || dpad == DPAD_E || dpad == DPAD_SE) controlData.buttons |= SCE_CTRL_RIGHT;
    if (dpad == DPAD_SE || dpad == DPAD_S || dpad == DPAD_SW) controlData.buttons |= SCE_CTRL_DOWN;
    if (dpad == DPAD_SW || dpad == DPAD_W || dpad == DPAD_NW) controlData.buttons |= SCE_CTRL_LEFT;

    // Face buttons (by physical position)
    if (btn1 & 0x02) controlData.buttons |= SCE_CTRL_CROSS;    // bottom
    if (btn1 & 0x01) controlData.buttons |= SCE_CTRL_CIRCLE;   // right
    if (btn1 & 0x10) controlData.buttons |= SCE_CTRL_SQUARE;   // left
    if (btn1 & 0x08) controlData.buttons |= SCE_CTRL_TRIANGLE; // top

    // Shoulders and triggers
    if (btn1 & 0x40) controlData.buttons |= SCE_CTRL_L1;
    if (btn1 & 0x80) controlData.buttons |= SCE_CTRL_R1;
    if (btn2 & 0x01) controlData.buttons |= SCE_CTRL_LTRIGGER;
    if (btn2 & 0x02) controlData.buttons |= SCE_CTRL_RTRIGGER;

    // System buttons and stick clicks
    if (btn2 & 0x04) controlData.buttons |= SCE_CTRL_SELECT;
    if (btn2 & 0x08) controlData.buttons |= SCE_CTRL_START;
    if (btn2 & 0x10) controlData.buttons |= SCE_CTRL_PSBUTTON;
    if (btn2 & 0x20) controlData.buttons |= SCE_CTRL_L3;
    if (btn2 & 0x40) controlData.buttons |= SCE_CTRL_R3;

    // Sticks
    controlData.leftX  = filterDeadzone(buffer[2]);
    controlData.leftY  = filterDeadzone(buffer[3]);
    controlData.rightX = filterDeadzone(buffer[4]);
    controlData.rightY = filterDeadzone(buffer[5]);
}
