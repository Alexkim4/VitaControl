#include <psp2kern/ctrl.h>
#include "eightbitdo_controller.h"

EightBitDoController::EightBitDoController(uint32_t mac0, uint32_t mac1, int port): Controller(mac0, mac1, port)
{
    // 입력 리포트 응답 유도를 위한 초기 빈 쓰기 요청
    static uint8_t report[4] = {};
    requestReport(HID_REQUEST_WRITE, report, sizeof(report));
}

void EightBitDoController::processReport(uint8_t *buffer, size_t length)
{
    if (length < 6) return;

    // Report ID(0x01)가 포함되어 들어오는 경우 오프셋 보정
    size_t offset = (buffer[0] == 0x01 && length >= 7) ? 1 : 0;
    EightBitDoDInputReport *report = (EightBitDoDInputReport*)(buffer + offset);

    controlData.buttons = 0;

    // 버튼 매핑 (Cross: 하단, Circle: 우측, Triangle: 상단, Square: 좌측)
    if (report->b) controlData.buttons |= SCE_CTRL_CROSS;
    if (report->a) controlData.buttons |= SCE_CTRL_CIRCLE;
    if (report->x) controlData.buttons |= SCE_CTRL_TRIANGLE;
    if (report->y) controlData.buttons |= SCE_CTRL_SQUARE;

    // D-Pad (Hat switch) 매핑
    int dpad = report->dpad;
    if (dpad == DPAD_NW || dpad == DPAD_N || dpad == DPAD_NE)
        controlData.buttons |= SCE_CTRL_UP;
    if (dpad == DPAD_NE || dpad == DPAD_E || dpad == DPAD_SE)
        controlData.buttons |= SCE_CTRL_RIGHT;
    if (dpad == DPAD_SE || dpad == DPAD_S || dpad == DPAD_SW)
        controlData.buttons |= SCE_CTRL_DOWN;
    if (dpad == DPAD_SW || dpad == DPAD_W || dpad == DPAD_NW)
        controlData.buttons |= SCE_CTRL_LEFT;

    // 범퍼 및 트리거
    if (report->l1) controlData.buttons |= SCE_CTRL_L1;
    if (report->r1) controlData.buttons |= SCE_CTRL_R1;
    if (report->l2) controlData.buttons |= SCE_CTRL_LTRIGGER;
    if (report->r2) controlData.buttons |= SCE_CTRL_RTRIGGER;
    if (report->stickL) controlData.buttons |= SCE_CTRL_L3;
    if (report->stickR) controlData.buttons |= SCE_CTRL_R3;

    // 시스템 및 메뉴 버튼
    if (report->start)  controlData.buttons |= SCE_CTRL_START;
    if (report->select) controlData.buttons |= SCE_CTRL_SELECT;
    if (report->home)   controlData.buttons |= SCE_CTRL_PSBUTTON;

    // 아날로그 스틱
    controlData.leftX  = report->leftX;
    controlData.leftY  = report->leftY;
    controlData.rightX = report->rightX;
    controlData.rightY = report->rightY;
}
