#include "fsm_tuning.h"
#include "fsm_automatic.h"
#include "global.h"
#include "software_timer.h"
#include "i2c-lcd.h"

enum { MIN_SEC = 1, MAX_SEC = 99, BLINK_MS = 250, MESSAGE_MS = 3000 };

/* Amber editing is a substate of TUNING_RED. Red moves, green stays fixed. */
static uint8_t editing_amber;
static uint8_t field_visible;

static void put_seconds(char *field, uint32_t seconds) {
    field[0] = (char)('0' + seconds / 10);
    field[1] = (char)('0' + seconds % 10);
}

static void show_draft(void) {
    char line[] = " R00  G00  A--";
    put_seconds(&line[2], temp_time_red);
    put_seconds(&line[7], temp_time_green);
    if (temp_time_red > temp_time_green) {
        put_seconds(&line[12], temp_time_red - temp_time_green);
    }
    int field = editing_amber ? 10 : (status == TUNING_GREEN ? 5 : 0);
    line[field] = '>';
    if (!field_visible) {
        line[field + 2] = ' ';
        line[field + 3] = ' ';
    }
    lcd_show_line(2, line);
}

static void show_selection(void) {
    field_visible = 1;
    setTimer(TIMER_BLINK, BLINK_MS);
    show_draft();
}

static uint8_t adjust_draft(int change) {
    if (change == 0) {
        return 0;
    }
    uint32_t *value = (status == TUNING_GREEN) ? &temp_time_green : &temp_time_red;
    int next = (int)*value + change;
    if (editing_amber && next <= (int)temp_time_green) {
        if (change < 0) {
            return 0;
        }
        next = (int)temp_time_green + 1;  /* Repair an undefined amber with +. */
    }
    if (next < MIN_SEC || next > MAX_SEC) {
        return 0;
    }
    *value = (uint32_t)next;
    return 1;
}

static void save_or_reject(void) {
    if (temp_time_green < MIN_SEC || temp_time_red > MAX_SEC ||
        temp_time_red <= temp_time_green) {
        lcd_show_line(2, "ERR: NEED R>G");
    } else {
        /* The cooperative tasks cannot see a partially updated configuration. */
        time_red = temp_time_red;
        time_green = temp_time_green;
        time_amber = temp_time_red - temp_time_green;
        lcd_show_line(2, "SAVED! A = R - G");
        if (lcd_line1_mode == LINE1_CONFIG) {
            lcd_display_config(time_red, time_green, time_amber);
        }
    }
    setTimer(TIMER_BLINK, 0);
    setTimer(TIMER_MSG, MESSAGE_MS);
    status = TUNING_MSG;
}

void fsm_tuning_run(void) {
    if (status == TUNING_INIT) {
        temp_time_red = time_red;
        temp_time_green = time_green;
        editing_amber = 0;
        fsm_reset_button_repeats();
        status = TUNING_RED;
        show_selection();
        return;
    }
    if (status != TUNING_RED && status != TUNING_GREEN && status != TUNING_MSG) {
        return;
    }
    FsmButtons buttons;
    fsm_read_buttons(&buttons);

    if (status == TUNING_MSG) {
        if (buttons.pressed[BTN_MODE] || isTimerEnd(TIMER_MSG)) {
            setTimer(TIMER_MSG, 0);
            fsm_automatic_resume();
        }
        return;
    }
    if (buttons.pressed[BTN_MODE]) {
        save_or_reject();
        return;
    }
    if (buttons.held[BTN_SELECT] && !editing_amber) {
        status = TUNING_RED;
        editing_amber = 1;
        show_selection();
    } else if (buttons.pressed[BTN_SELECT]) {
        status = (editing_amber || status == TUNING_GREEN) ? TUNING_RED : TUNING_GREEN;
        editing_amber = 0;
        show_selection();
    } else if (adjust_draft(fsm_adjustment(&buttons))) {
        show_selection();
    } else if (isTimerEnd(TIMER_BLINK)) {
        field_visible = !field_visible;
        setTimer(TIMER_BLINK, BLINK_MS);
        show_draft();
    }
}
