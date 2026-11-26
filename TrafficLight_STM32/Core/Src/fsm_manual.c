/*
 * fsm_manual.c - Hiện thực FSM Chế độ Bằng tay (Manual Mode)
 * LƯU Ý: Tuyệt đối KHÔNG gọi lệnh HAL (HAL_GPIO, HAL_I2C, HAL_Delay) trong file này.
 */

#include "fsm_manual.h"
#include "fsm_automatic.h"
#include "global.h"
#include "button.h"
#include "traffic_light.h"
#include "i2c-lcd.h"

static void apply_manual_phase(uint8_t l1, uint8_t l2, const char *lcd_str, int next_status) {
    traffic_set(LANE_1, l1);
    traffic_set(LANE_2, l2);
    lcd_show_line(2, lcd_str);
    status = next_status;
}

void fsm_manual_enter(int target_status) {
    // Xả cờ nút cũ trước khi vào Manual
    (void)isPressed(BTN_SELECT);
    (void)isPressed(BTN_INC);

    switch (target_status) {
        case MAN_RED1_GREEN2:
            apply_manual_phase(RED, GREEN, "M 1:RED  2:GREEN", MAN_RED1_GREEN2);
            break;
        case MAN_RED1_AMBER2:
            apply_manual_phase(RED, AMBER, "M 1:RED  2:AMBER", MAN_RED1_AMBER2);
            break;
        case MAN_GREEN1_RED2:
            apply_manual_phase(GREEN, RED, "M 1:GREEN  2:RED", MAN_GREEN1_RED2);
            break;
        case MAN_AMBER1_RED2:
            apply_manual_phase(AMBER, RED, "M 1:AMBER  2:RED", MAN_AMBER1_RED2);
            break;
        default:
            apply_manual_phase(RED, GREEN, "M 1:RED  2:GREEN", MAN_RED1_GREEN2);
            break;
    }
}

static void handle_manual_case(uint8_t next_l1, uint8_t next_l2, const char *next_lcd,
                               int next_man_status, int next_auto_status) {
    if (isPressed(BTN_MODE)) {
        // Bật Auto chạy nền ngay từ pha hiện tại trước khi vào Tuning
        int bg_phase = AUTO_RED1_GREEN2;
        switch (status) {
            case MAN_RED1_GREEN2: bg_phase = AUTO_RED1_GREEN2; break;
            case MAN_RED1_AMBER2: bg_phase = AUTO_RED1_AMBER2; break;
            case MAN_GREEN1_RED2: bg_phase = AUTO_GREEN1_RED2; break;
            case MAN_AMBER1_RED2: bg_phase = AUTO_AMBER1_RED2; break;
            default: break;
        }
        fsm_automatic_start_bg(bg_phase);
        status = TUNING_INIT;
        return;
    }
    if (isPressed(BTN_INC)) {
        fsm_automatic_enter(next_auto_status);
        return;
    }
    if (isPressed(BTN_SELECT)) {
        apply_manual_phase(next_l1, next_l2, next_lcd, next_man_status);
    }
}

void fsm_manual_run(void) {
    switch (status) {
        case MAN_INIT:
            fsm_manual_enter(MAN_RED1_GREEN2);
            break;

        case MAN_RED1_GREEN2:
            handle_manual_case(RED, AMBER, "M 1:RED  2:AMBER",
                               MAN_RED1_AMBER2, AUTO_RED1_AMBER2);
            break;

        case MAN_RED1_AMBER2:
            handle_manual_case(GREEN, RED, "M 1:GREEN  2:RED",
                               MAN_GREEN1_RED2, AUTO_GREEN1_RED2);
            break;

        case MAN_GREEN1_RED2:
            handle_manual_case(AMBER, RED, "M 1:AMBER  2:RED",
                               MAN_AMBER1_RED2, AUTO_AMBER1_RED2);
            break;

        case MAN_AMBER1_RED2:
            handle_manual_case(RED, GREEN, "M 1:RED  2:GREEN",
                               MAN_RED1_GREEN2, AUTO_RED1_GREEN2);
            break;

        default:
            break;
    }
}
