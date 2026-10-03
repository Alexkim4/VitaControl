#include <psp2kern/ctrl.h>
#include "eightbitdo_controller.h"

static inline uint8_t filterDeadzone(uint8_t val)
{
    if (val >= 123 && val <= 133)
        return 128;
    return val;
}

EightBitDoController::EightBitDoController(uint32_t mac0, uint32_t mac1, int port): Controller(mac0, mac1, port)
{
    // 8BitDo D-mode HID 리포트 전송을 활성화하기 위한 초기 빈 쓰기 요청
    static uint8_t report[4] = {};
    requestReport(HID_REQUEST_WRITE, report, sizeof(report));
}

void EightBitDoController::processReport(uint8_t *buffer, size_t length)
{
    if (length < 7) return;

    controlData.buttons = 0;

    // 포맷 1: Report ID 0x01 표준 블루투스 HID 리포트 (8BitDo Pro 2 / Pro 3 / SN30 Pro 등)
    if (buffer[0] == 0x01 && length >= 10)
    {
        uint8_t dpad     = buffer[1] & 0x0F;
        uint8_t leftX    = filterDeadzone(buffer[2]);
        uint8_t leftY    = filterDeadzone(buffer[3]);
        uint8_t rightX   = filterDeadzone(buffer[4]);
        uint8_t rightY   = filterDeadzone(buffer[5]);
        uint8_t triggerR = buffer[6];
        uint8_t triggerL = buffer[7];
        uint8_t btn1     = buffer[8];
        uint8_t btn2     = buffer[9];

        // D-Pad Hat Switch (0: 상, 1: 우상, 2: 우, 3: 우하, 4: 하, 5: 좌하, 6: 좌, 7: 좌상, 8~15: 중립)
        if (dpad == 7 || dpad == 0 || dpad == 1) controlData.buttons |= SCE_CTRL_UP;
        if (dpad == 1 || dpad == 2 || dpad == 3) controlData.buttons |= SCE_CTRL_RIGHT;
        if (dpad == 3 || dpad == 4 || dpad == 5) controlData.buttons |= SCE_CTRL_DOWN;
        if (dpad == 5 || dpad == 6 || dpad == 7) controlData.buttons |= SCE_CTRL_LEFT;

        // 전면 버튼 (B=Cross, A=Circle, Y=Square, X=Triangle)
        if (btn1 & 0x01) controlData.buttons |= SCE_CTRL_CROSS;
        if (btn1 & 0x02) controlData.buttons |= SCE_CTRL_CIRCLE;
        if (btn1 & 0x08) controlData.buttons |= SCE_CTRL_SQUARE;
        if (btn1 & 0x10) controlData.buttons |= SCE_CTRL_TRIANGLE;

        // 숄더 범퍼 (L1 / R1)
        if (btn1 & 0x40) controlData.buttons |= SCE_CTRL_L1;
        if (btn1 & 0x80) controlData.buttons |= SCE_CTRL_R1;

        // 아날로그 트리거 (L2 / R2)
        if (triggerL > 30) controlData.buttons |= SCE_CTRL_LTRIGGER;
        if (triggerR > 30) controlData.buttons |= SCE_CTRL_RTRIGGER;

        // 시스템 및 스틱 클릭
        if (btn2 & 0x04) controlData.buttons |= SCE_CTRL_SELECT;
        if (btn2 & 0x08) controlData.buttons |= SCE_CTRL_START;
        if (btn2 & 0x10) controlData.buttons |= SCE_CTRL_PSBUTTON;
        if (btn2 & 0x20) controlData.buttons |= SCE_CTRL_L3;
        if (btn2 & 0x40) controlData.buttons |= SCE_CTRL_R3;

        // 아날로그 스틱
        controlData.leftX  = leftX;
        controlData.leftY  = leftY;
        controlData.rightX = rightX;
        controlData.rightY = rightY;

        // 배터리 잔량 (14번 바이트)
        if (length >= 15)
        {
            uint8_t batt = buffer[14] & 0x7F;
            if (batt > 0 && batt <= 100)
                batteryLevel = (batt * 5) / 100;
        }
    }
    // 포맷 2: Report ID가 생략된 원시 블루투스 리포트 (dpad가 buffer[0]에 위치)
    else if (buffer[0] <= 8 && length >= 9 && buffer[1] >= 30 && buffer[1] <= 225 && buffer[2] >= 30 && buffer[2] <= 225)
    {
        uint8_t dpad     = buffer[0] & 0x0F;
        uint8_t leftX    = filterDeadzone(buffer[1]);
        uint8_t leftY    = filterDeadzone(buffer[2]);
        uint8_t rightX   = filterDeadzone(buffer[3]);
        uint8_t rightY   = filterDeadzone(buffer[4]);
        uint8_t triggerR = buffer[5];
        uint8_t triggerL = buffer[6];
        uint8_t btn1     = buffer[7];
        uint8_t btn2     = buffer[8];

        if (dpad == 7 || dpad == 0 || dpad == 1) controlData.buttons |= SCE_CTRL_UP;
        if (dpad == 1 || dpad == 2 || dpad == 3) controlData.buttons |= SCE_CTRL_RIGHT;
        if (dpad == 3 || dpad == 4 || dpad == 5) controlData.buttons |= SCE_CTRL_DOWN;
        if (dpad == 5 || dpad == 6 || dpad == 7) controlData.buttons |= SCE_CTRL_LEFT;

        if (btn1 & 0x01) controlData.buttons |= SCE_CTRL_CROSS;
        if (btn1 & 0x02) controlData.buttons |= SCE_CTRL_CIRCLE;
        if (btn1 & 0x08) controlData.buttons |= SCE_CTRL_SQUARE;
        if (btn1 & 0x10) controlData.buttons |= SCE_CTRL_TRIANGLE;

        if (btn1 & 0x40) controlData.buttons |= SCE_CTRL_L1;
        if (btn1 & 0x80) controlData.buttons |= SCE_CTRL_R1;

        if (triggerL > 30) controlData.buttons |= SCE_CTRL_LTRIGGER;
        if (triggerR > 30) controlData.buttons |= SCE_CTRL_RTRIGGER;

        if (btn2 & 0x04) controlData.buttons |= SCE_CTRL_SELECT;
        if (btn2 & 0x08) controlData.buttons |= SCE_CTRL_START;
        if (btn2 & 0x10) controlData.buttons |= SCE_CTRL_PSBUTTON;
        if (btn2 & 0x20) controlData.buttons |= SCE_CTRL_L3;
        if (btn2 & 0x40) controlData.buttons |= SCE_CTRL_R3;

        controlData.leftX  = leftX;
        controlData.leftY  = leftY;
        controlData.rightX = rightX;
        controlData.rightY = rightY;
    }
    // 포맷 3: 구형 펌웨어/USB 리포트 폴백
    else
    {
        uint8_t btn1   = buffer[0];
        uint8_t btn2   = buffer[1];
        uint8_t dpad   = buffer[2] & 0x0F;
        uint8_t leftX  = filterDeadzone(buffer[3]);
        uint8_t leftY  = filterDeadzone(buffer[4]);
        uint8_t rightX = filterDeadzone((length >= 6) ? buffer[5] : 128);
        uint8_t rightY = filterDeadzone((length >= 7) ? buffer[6] : 128);

        if (dpad == 7 || dpad == 0 || dpad == 1) controlData.buttons |= SCE_CTRL_UP;
        if (dpad == 1 || dpad == 2 || dpad == 3) controlData.buttons |= SCE_CTRL_RIGHT;
        if (dpad == 3 || dpad == 4 || dpad == 5) controlData.buttons |= SCE_CTRL_DOWN;
        if (dpad == 5 || dpad == 6 || dpad == 7) controlData.buttons |= SCE_CTRL_LEFT;

        if (btn1 & 0x01) controlData.buttons |= SCE_CTRL_CROSS;
        if (btn1 & 0x02) controlData.buttons |= SCE_CTRL_CIRCLE;
        if (btn1 & 0x08) controlData.buttons |= SCE_CTRL_SQUARE;
        if (btn1 & 0x10) controlData.buttons |= SCE_CTRL_TRIANGLE;

        if (btn1 & 0x40) controlData.buttons |= SCE_CTRL_L1;
        if (btn1 & 0x80) controlData.buttons |= SCE_CTRL_R1;

        if (btn2 & 0x01) controlData.buttons |= SCE_CTRL_LTRIGGER;
        if (btn2 & 0x02) controlData.buttons |= SCE_CTRL_RTRIGGER;
        if (btn2 & 0x04) controlData.buttons |= SCE_CTRL_SELECT;
        if (btn2 & 0x08) controlData.buttons |= SCE_CTRL_START;
        if (btn2 & 0x10) controlData.buttons |= SCE_CTRL_PSBUTTON;
        if (btn2 & 0x20) controlData.buttons |= SCE_CTRL_L3;
        if (btn2 & 0x40) controlData.buttons |= SCE_CTRL_R3;

        controlData.leftX  = leftX;
        controlData.leftY  = leftY;
        controlData.rightX = rightX;
        controlData.rightY = rightY;
    }
}
