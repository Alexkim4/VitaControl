#include <psp2kern/ctrl.h>
#include "eightbitdo_controller.h"

static inline uint8_t filterDeadzone(uint8_t val)
{
    if (val >= 118 && val <= 138)
        return 128;
    return val;
}

EightBitDoController::EightBitDoController(uint32_t mac0, uint32_t mac1, int port): Controller(mac0, mac1, port)
{
    // Send an initial empty write request to prompt 8BitDo controller to stream input reports
    static uint8_t report[4] = {};
    requestReport(HID_REQUEST_WRITE, report, sizeof(report));
}

void EightBitDoController::processReport(uint8_t *buffer, size_t length)
{
    // 0x01 기본 입력 리포트가 아니거나 길이가 부족하면 무시 (버튼 상태 유지)
    if (buffer[0] != 0x01 || length < 10)
        return;

    // 정상 입력 패킷에 대해서만 버튼 초기화
    controlData.buttons = 0;

    uint8_t dpad     = buffer[1] & 0x0F;
    uint8_t leftX    = filterDeadzone(buffer[2]);
    uint8_t leftY    = filterDeadzone(buffer[3]);
    uint8_t rightX   = filterDeadzone(buffer[4]);
    uint8_t rightY   = filterDeadzone(buffer[5]);
    uint8_t triggerR = buffer[6];
    uint8_t triggerL = buffer[7];
    uint8_t btn1     = buffer[8];
    uint8_t btn2     = buffer[9];

    // D-Pad Hat Switch (0=상, 1=우상, 2=우, 3=우하, 4=하, 5=좌하, 6=좌, 7=좌상, 8=중립)
    if (dpad == 7 || dpad == 0 || dpad == 1) controlData.buttons |= SCE_CTRL_UP;
    if (dpad == 1 || dpad == 2 || dpad == 3) controlData.buttons |= SCE_CTRL_RIGHT;
    if (dpad == 3 || dpad == 4 || dpad == 5) controlData.buttons |= SCE_CTRL_DOWN;
    if (dpad == 5 || dpad == 6 || dpad == 7) controlData.buttons |= SCE_CTRL_LEFT;

    // 전면 버튼 (8BitDo D-Input / SDL 표준 물리 배치에 맞춤)
    if (btn1 & 0x01) controlData.buttons |= SCE_CTRL_CIRCLE;    // 물리 우측 (A) -> Circle (○)
    if (btn1 & 0x02) controlData.buttons |= SCE_CTRL_CROSS;     // 물리 하단 (B) -> Cross (✕)
    if (btn1 & 0x08) controlData.buttons |= SCE_CTRL_TRIANGLE;  // 물리 상단 (X) -> Triangle (△)
    if (btn1 & 0x10) controlData.buttons |= SCE_CTRL_SQUARE;    // 물리 좌측 (Y) -> Square (□)

    // 범퍼 (L1 / R1)
    if (btn1 & 0x40) controlData.buttons |= SCE_CTRL_L1;
    if (btn1 & 0x80) controlData.buttons |= SCE_CTRL_R1;

    // 트리거 (디지털 비트 및 아날로그 트리거 모두 지원)
    if ((btn2 & 0x01) || triggerL > 30) controlData.buttons |= SCE_CTRL_LTRIGGER;
    if ((btn2 & 0x02) || triggerR > 30) controlData.buttons |= SCE_CTRL_RTRIGGER;

    // 시스템 버튼 및 스틱 클릭 (L3 / R3)
    if (btn2 & 0x04) controlData.buttons |= SCE_CTRL_SELECT;
    if (btn2 & 0x08) controlData.buttons |= SCE_CTRL_START;
    if (btn2 & 0x10) controlData.buttons |= SCE_CTRL_PSBUTTON;
    if (btn2 & 0x20) controlData.buttons |= SCE_CTRL_L3;
    if (btn2 & 0x40) controlData.buttons |= SCE_CTRL_R3;

    // 아날로그 스틱 (128 중립 필터 적용)
    controlData.leftX  = leftX;
    controlData.leftY  = leftY;
    controlData.rightX = rightX;
    controlData.rightY = rightY;

    // 배터리 잔량 보고 (14번째 바이트)
    if (length >= 15)
    {
        uint8_t batt = buffer[14] & 0x7F;
        if (batt > 0 && batt <= 100)
            batteryLevel = (batt * 5) / 100;
    }
}
