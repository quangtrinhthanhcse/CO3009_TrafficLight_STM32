/*
 * fsm_automatic.c - Hiện thực FSM Chế độ Tự động (Automatic Mode)
 * LƯU Ý: Tuyệt đối KHÔNG gọi lệnh HAL (HAL_GPIO, HAL_I2C, HAL_Delay) trong file này.
 */

#include "fsm_automatic.h"
#include "fsm_manual.h"
#include <stdio.h>
#include "global.h"
#include "software_timer.h"
#include "button.h"
#include "traffic_light.h"
#include "i2c-lcd.h"
#include "dht20.h"

#define NIGHT_TIME_MIN      1
#define NIGHT_TIME_MAX      99
#define NIGHT_TIME_DEFAULT  10
#define NIGHT_CFG_BLINK_MS  500   // Nháy Vàng khi chỉnh giờ (1 Hz)
#define NIGHT_RUN_ON_MS     2000  // Sáng Vàng 2s, tắt 1s khi chạy Night Mode
#define NIGHT_RUN_OFF_MS    1000

static uint32_t counter_l1 = 0;
static uint32_t counter_l2 = 0;
static int      bg_auto_status = AUTO_RED1_GREEN2; // Lưu pha Auto để chạy nền khi Tuning

static uint8_t  night_blink_state = 0;
static uint32_t night_timer = NIGHT_TIME_DEFAULT;

static void show_auto_lcd(char c1, uint32_t t1, char c2, uint32_t t2) {
    char buf[17];
    snprintf(buf, sizeof(buf), "A 1:%c%02lu  2:%c%02lu",
             c1, (unsigned long)t1, c2, (unsigned long)t2);
    lcd_show_line(2, buf);
}

static void show_night_lcd(uint32_t hour) {
    char buf[17];
    snprintf(buf, sizeof(buf), "NIGHT_MODE: %02luh", (unsigned long)hour);
    lcd_show_line(1, buf);
}

static void exit_night_to_auto(void) {
    lcd_show_line(1, "                "); // Xóa dòng 1 để task_dht20_lcd ghi lại nhiệt độ/độ ẩm
    fsm_automatic_enter(AUTO_RED1_GREEN2);
}

static void blink_night_amber(uint32_t on_ms, uint32_t off_ms) {
    if (isTimerEnd(TIMER_BLINK)) {
        night_blink_state = !night_blink_state;
        setTimer(TIMER_BLINK, night_blink_state ? on_ms : off_ms);
        uint8_t out = night_blink_state ? AMBER : OFF;

        traffic_set(LANE_1, out);
        traffic_set(LANE_2, out);
    }
}

static void refresh_auto_lcd(int phase) {
    switch (phase) {
        case AUTO_RED1_GREEN2: show_auto_lcd('R', counter_l1, 'G', counter_l2); break;
        case AUTO_RED1_AMBER2: show_auto_lcd('R', counter_l1, 'A', counter_l2); break;
        case AUTO_GREEN1_RED2: show_auto_lcd('G', counter_l1, 'R', counter_l2); break;
        case AUTO_AMBER1_RED2: show_auto_lcd('A', counter_l1, 'R', counter_l2); break;
        default: break;
    }
}

// Nạp một pha Auto; truyền update_lcd = 0 khi chạy nền trong Tuning
static uint8_t load_auto_phase(int target_status, uint8_t update_lcd) {
    switch (target_status) {
        case AUTO_RED1_GREEN2:
            counter_l1 = time_red;
            counter_l2 = time_green;
            traffic_set(LANE_1, RED);
            traffic_set(LANE_2, GREEN);
            break;

        case AUTO_RED1_AMBER2:
            counter_l1 = time_amber;
            counter_l2 = time_amber;
            traffic_set(LANE_1, RED);
            traffic_set(LANE_2, AMBER);
            break;

        case AUTO_GREEN1_RED2:
            counter_l1 = time_green;
            counter_l2 = time_red;
            traffic_set(LANE_1, GREEN);
            traffic_set(LANE_2, RED);
            break;

        case AUTO_AMBER1_RED2:
            counter_l1 = time_amber;
            counter_l2 = time_amber;
            traffic_set(LANE_1, AMBER);
            traffic_set(LANE_2, RED);
            break;

        default:
            return 0;
    }
    bg_auto_status = target_status;
    if (update_lcd) {
        refresh_auto_lcd(target_status);
    }
    setTimer(TIMER_TRAFFIC, 1000);
    return 1;
}

static void step_auto_timer(uint8_t update_lcd) {
    if (!isTimerEnd(TIMER_TRAFFIC)) {
        return;
    }

    counter_l1--;
    counter_l2--;

    int next_phase = 0;
    if (bg_auto_status == AUTO_RED1_GREEN2 && counter_l2 == 0) {
        next_phase = AUTO_RED1_AMBER2;
    } else if (bg_auto_status == AUTO_RED1_AMBER2 && counter_l2 == 0) {
        next_phase = AUTO_GREEN1_RED2;
    } else if (bg_auto_status == AUTO_GREEN1_RED2 && counter_l1 == 0) {
        next_phase = AUTO_AMBER1_RED2;
    } else if (bg_auto_status == AUTO_AMBER1_RED2 && counter_l1 == 0) {
        next_phase = AUTO_RED1_GREEN2;
    }

    if (next_phase != 0) {
        load_auto_phase(next_phase, update_lcd);
        if (update_lcd) {
            status = next_phase;
        }
    } else {
        setTimer(TIMER_TRAFFIC, 1000);
        if (update_lcd) {
            refresh_auto_lcd(bg_auto_status);
        }
    }
}

void fsm_automatic_enter(int target_status) {
    if (!load_auto_phase(target_status, 1)) {
        return;
    }
    (void)isLongPressed(BTN_MODE);
    status = target_status;

    (void)isPressed(BTN_DEC);

}

void fsm_automatic_start_bg(int target_status) {
    (void)load_auto_phase(target_status, 0);
}

void fsm_automatic_resume(void) {
    (void)isLongPressed(BTN_MODE);
    (void)isPressed(BTN_DEC);
    status = bg_auto_status;
    refresh_auto_lcd(bg_auto_status);
}

void fsm_automatic_run(void) {
    // Trong lúc Tuning, đèn ngoài đường vẫn chạy Auto ngầm nhưng không đè LCD dòng 2
    if (status >= TUNING_INIT && status <= TUNING_MSG) {
        step_auto_timer(0);
        return;
    }

    // BTN_1 nhấn ngắn -> đóng băng sang Manual; nhấn giữ -> hẹn giờ Night Mode
    if (status >= AUTO_RED1_GREEN2 && status <= AUTO_AMBER1_RED2) {
        if (isPressed(BTN_MODE)) {
            int target_man = MAN_RED1_GREEN2;
            switch (status) {
                case AUTO_RED1_GREEN2: target_man = MAN_RED1_GREEN2; break;
                case AUTO_RED1_AMBER2: target_man = MAN_RED1_AMBER2; break;
                case AUTO_GREEN1_RED2: target_man = MAN_GREEN1_RED2; break;
                case AUTO_AMBER1_RED2: target_man = MAN_AMBER1_RED2; break;
                default: break;
            }
            fsm_manual_enter(target_man);
            return;
        }
        if (isLongPressed(BTN_MODE)) {
            status = NIGHT_INIT;
            return;
        }
        if (isPressed(BTN_DEC)){
			lcd_line1_mode = (lcd_line1_mode == LINE1_DHT20) ? LINE1_CONFIG : LINE1_DHT20;
			if (lcd_line1_mode == LINE1_DHT20) lcd_display_env(value_x10[1], value_x10[0], dht20_is_mock());
			else lcd_display_config(time_red, time_green, time_amber);
		}
        step_auto_timer(1);
        return;
    }

    switch (status) {
        case INIT:
            fsm_automatic_enter(AUTO_RED1_GREEN2);
            break;

        case NIGHT_INIT:
            // Xả cờ nút cũ trước khi chỉnh hẹn giờ
            (void)isPressed(BTN_MODE);
            (void)isLongPressed(BTN_MODE);
            (void)isPressed(BTN_SELECT);
            (void)isPressed(BTN_INC);
            (void)isLongPressed(BTN_INC);
            (void)isPressed(BTN_DEC);
            (void)isLongPressed(BTN_DEC);

            night_blink_state = 1;
            traffic_set(LANE_1, AMBER);
            traffic_set(LANE_2, AMBER);
            setTimer(TIMER_BLINK, NIGHT_CFG_BLINK_MS);

            night_timer = NIGHT_TIME_DEFAULT;
            show_night_lcd(night_timer);
            lcd_show_line(2, "BTN3/4:+/- 2:OK");
            status = NIGHT_CONFIG;
            break;

        case NIGHT_CONFIG:
            blink_night_amber(NIGHT_CFG_BLINK_MS, NIGHT_CFG_BLINK_MS);
            if ((isPressed(BTN_INC) || isLongPressed(BTN_INC)) && (night_timer < NIGHT_TIME_MAX)) {
                night_timer++;
                show_night_lcd(night_timer);
            }
            if ((isPressed(BTN_DEC) || isLongPressed(BTN_DEC)) && (night_timer > NIGHT_TIME_MIN)) {
                night_timer--;
                show_night_lcd(night_timer);
            }
            if (isPressed(BTN_SELECT)) {
                night_blink_state = 1;
                traffic_set(LANE_1, AMBER);
                traffic_set(LANE_2, AMBER);
                show_night_lcd(night_timer);
                lcd_show_line(2, "RUNNING (1:EXIT)");
                setTimer(TIMER_BLINK, NIGHT_RUN_ON_MS);
                setTimer(TIMER_TRAFFIC, 1000);
                status = NIGHT_BLINK;
                break;
            }
            if (isPressed(BTN_MODE)) {
                exit_night_to_auto();
            }
            break;

        case NIGHT_BLINK:
            if (isLongPressed(BTN_MODE) || isPressed(BTN_MODE)) {
                exit_night_to_auto();
                break;
            }

            blink_night_amber(NIGHT_RUN_ON_MS, NIGHT_RUN_OFF_MS);

            if (isTimerEnd(TIMER_TRAFFIC)) {
                night_timer--;
                if (night_timer == 0) {
                    exit_night_to_auto();
                } else {
                    setTimer(TIMER_TRAFFIC, 1000);
                    show_night_lcd(night_timer);
                }
            }
            break;

        default:
            break;
    }
}
