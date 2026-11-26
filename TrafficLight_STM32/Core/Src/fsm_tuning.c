/*
 * fsm_tuning.c - Hiện thực FSM Chế độ Chỉnh thời gian (Tuning Mode)
 * LƯU Ý: Tuyệt đối KHÔNG gọi lệnh HAL (HAL_GPIO, HAL_I2C, HAL_Delay) trong file này.
 */

#include "fsm_tuning.h"
#include "fsm_automatic.h"
#include <stdio.h>
#include "global.h"
#include "software_timer.h"
#include "button.h"
#include "i2c-lcd.h"

#define TIME_MIN  1
#define TIME_MAX  99

static uint8_t blink_state = 0;

// Hiển thị dòng 2 LCD kèm Vàng tạm tính (A = R - G); show_field = 0 để tạo hiệu ứng nháy cụm đang chỉnh
static void show_tuning_lcd(char mode_char, uint32_t val, uint8_t show_field) {
    char buf[17];
    if (temp_time_red > temp_time_green) {
        uint32_t temp_amber = temp_time_red - temp_time_green;
        char condition = (temp_amber >= 3 && temp_time_green >= temp_amber) ? 's' : '!';
        if (show_field) {
            snprintf(buf, sizeof(buf), "TUNE %c:%02lus A:%02lu%c",
                     mode_char, (unsigned long)val, (unsigned long)temp_amber, condition);
        } else {
            snprintf(buf, sizeof(buf), "TUNE       A:%02lu%c",
                     (unsigned long)temp_amber, condition);
        }
    } else {
        if (show_field) {
            snprintf(buf, sizeof(buf), "TUNE %c:%02lus A:ERR",
                     mode_char, (unsigned long)val);
        } else {
            snprintf(buf, sizeof(buf), "TUNE       A:ERR");
        }
    }
    lcd_show_line(2, buf);
}

static void blink_tuning_lcd(char mode_char, uint32_t val) {
    if (isTimerEnd(TIMER_BLINK)) {
        setTimer(TIMER_BLINK, 250);
        blink_state = !blink_state;
        show_tuning_lcd(mode_char, val, blink_state);
    }
}

static void start_tuning_color(char mode_char, uint32_t val, int next_state) {
    blink_state = 1;
    show_tuning_lcd(mode_char, val, blink_state);
    setTimer(TIMER_BLINK, 250);
    status = next_state;
}

static uint8_t handle_inc_dec(uint32_t *val) {
    uint8_t changed = 0;
    if ((isPressed(BTN_INC) || isLongPressed(BTN_INC)) && (*val < TIME_MAX)) {
        (*val)++;
        changed = 1;
    }
    if ((isPressed(BTN_DEC) || isLongPressed(BTN_DEC)) && (*val > TIME_MIN)) {
        (*val)--;
        changed = 1;
    }
    return changed;
}

static void exit_tuning_and_validate(void) {
	if (temp_time_green >= temp_time_red) {
	        lcd_show_line(2, "ERR: G >= R!");
	    } else {
	        uint32_t temp_amber = temp_time_red - temp_time_green;
	        if (temp_amber < 3) {
	            lcd_show_line(2, "ERR: A < 3s!");
	        } else if (temp_time_green < temp_amber) {
	            lcd_show_line(2, "ERR: G < A!");
	        } else {
	            time_red   = temp_time_red;
	            time_green = temp_time_green;
	            time_amber = temp_amber;
	            lcd_show_line(2, "SAVED! A = R - G");
	        }
	    }
    setTimer(TIMER_MSG, 2500);
    status = TUNING_MSG;
}

void fsm_tuning_run(void) {
    switch (status) {
        case TUNING_INIT:
            temp_time_red   = time_red;
            temp_time_green = time_green;

            // Xả cờ nút cũ trước khi vào Tuning
            (void)isPressed(BTN_SELECT);
            (void)isPressed(BTN_INC);
            (void)isLongPressed(BTN_INC);
            (void)isPressed(BTN_DEC);
            (void)isLongPressed(BTN_DEC);

            start_tuning_color('R', temp_time_red, TUNING_RED);
            break;

        case TUNING_RED:
            blink_tuning_lcd('R', temp_time_red);
            if (handle_inc_dec(&temp_time_red)) {
                blink_state = 1;
                setTimer(TIMER_BLINK, 250);
                show_tuning_lcd('R', temp_time_red, 1);
            }
            if (isPressed(BTN_SELECT)) {
                start_tuning_color('G', temp_time_green, TUNING_GREEN);
            } else if (isPressed(BTN_MODE)) {
                exit_tuning_and_validate();
            }
            break;

        case TUNING_GREEN:
            blink_tuning_lcd('G', temp_time_green);
            if (handle_inc_dec(&temp_time_green)) {
                blink_state = 1;
                setTimer(TIMER_BLINK, 250);
                show_tuning_lcd('G', temp_time_green, 1);
            }
            if (isPressed(BTN_SELECT)) {
                start_tuning_color('R', temp_time_red, TUNING_RED);
            } else if (isPressed(BTN_MODE)) {
                exit_tuning_and_validate();
            }
            break;

        case TUNING_MSG:
            if (isTimerEnd(TIMER_MSG) || isPressed(BTN_MODE)) {
                fsm_automatic_resume();
            }
            break;

        default:
            break;
    }
}
