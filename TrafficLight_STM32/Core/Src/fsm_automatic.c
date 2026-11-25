#include "fsm_automatic.h"
#include "fsm_manual.h"
#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"
#include "dht20.h"
#include "i2c-lcd.h"
#include <stddef.h>
#include <stdio.h>

enum {
    SECOND_MS = 1000,
    NIGHT_DEFAULT_SEC = 10,
    NIGHT_MAX_SEC = 99,
    NIGHT_SETUP_BLINK_MS = 500,
    NIGHT_ON_MS = 2000,
    NIGHT_OFF_MS = 1000
};

static const FsmPhase phases[] = {
    {AUTO_RED1_GREEN2, MAN_RED1_GREEN2, LANE_2, GREEN},
    {AUTO_RED1_AMBER2, MAN_RED1_AMBER2, LANE_2, AMBER},
    {AUTO_GREEN1_RED2, MAN_GREEN1_RED2, LANE_1, GREEN},
    {AUTO_AMBER1_RED2, MAN_AMBER1_RED2, LANE_1, AMBER}
};
#define PHASE_COUNT (sizeof(phases) / sizeof(phases[0]))

static const FsmPhase *auto_phase = &phases[0];
static uint32_t active_left;
static uint32_t red_left;
static uint8_t night_seconds;
static uint8_t night_lit;

static uint8_t repeat_enabled[NUM_BUTTONS];

void fsm_reset_button_repeats(void) {
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        repeat_enabled[i] = 0;
    }
}

void fsm_read_buttons(FsmButtons *buttons) {
    for (int i = 0; i < NUM_BUTTONS; ++i) {
        buttons->pressed[i] = (uint8_t)isPressed(i);
        if (buttons->pressed[i]) {
            repeat_enabled[i] = 1;
        }
        int held = isLongPressed(i);
        buttons->held[i] = held && (i == BTN_MODE || repeat_enabled[i]);
    }
}

int fsm_adjustment(const FsmButtons *buttons) {
    int up = buttons->pressed[BTN_INC] || buttons->held[BTN_INC];
    int down = buttons->pressed[BTN_DEC] || buttons->held[BTN_DEC];
    return up - down;
}

const FsmPhase *fsm_find_phase(int status_code) {
    for (size_t i = 0; i < PHASE_COUNT; ++i) {
        if (phases[i].automatic == status_code || phases[i].manual == status_code) {
            return &phases[i];
        }
    }
    return NULL;
}

const FsmPhase *fsm_next_phase(const FsmPhase *phase) {
    return (phase == &phases[PHASE_COUNT - 1]) ? &phases[0] : phase + 1;
}

void fsm_apply_phase(const FsmPhase *phase) {
    uint8_t opposing_lane = (phase->active_lane == LANE_1) ? LANE_2 : LANE_1;
    traffic_set(opposing_lane, RED);
    traffic_set(phase->active_lane, phase->color);
}

static void show_auto(void) {
    char line[17];
    char active_color = (auto_phase->color == GREEN) ? 'G' : 'A';
    uint8_t lane1_active = (auto_phase->active_lane == LANE_1);
    unsigned t1 = (uint8_t)(lane1_active ? active_left : red_left);
    unsigned t2 = (uint8_t)(lane1_active ? red_left : active_left);
    snprintf(line, sizeof(line), "A 1:%c%02u 2:%c%02u",
             lane1_active ? active_color : 'R', t1,
             lane1_active ? 'R' : active_color, t2);
    lcd_show_line(2, line);
}

static void show_first_line(void) {
    if (lcd_line1_mode == LINE1_DHT20) {
        lcd_display_env(value_x10[1], value_x10[0], dht20_is_mock());
    } else {
        lcd_display_config(time_red, time_green, time_amber);
    }
}

static void load_phase(const FsmPhase *phase) {
    auto_phase = phase;
    fsm_apply_phase(phase);
    if (phase->color == GREEN) {
        active_left = time_green;
        red_left = time_red;
    } else {
        active_left = time_amber;
        red_left = time_amber;
    }
    setTimer(TIMER_TRAFFIC, SECOND_MS);
}

void fsm_automatic_enter(int target_status) {
    const FsmPhase *phase = fsm_find_phase(target_status);
    if (phase != NULL) {
        load_phase(phase);
        status = phase->automatic;
        show_auto();
    }
}

void fsm_automatic_start_bg(int target_status) {
    const FsmPhase *phase = fsm_find_phase(target_status);
    if (phase != NULL) {
        load_phase(phase);
    }
}

void fsm_automatic_resume(void) {
    status = auto_phase->automatic;
    show_auto();  /* Keep the counters and the fractional second already in progress. */
}

static void advance_clock(uint8_t foreground) {
    if (!isTimerEnd(TIMER_TRAFFIC)) {
        return;
    }
    if (active_left <= 1) {
        load_phase(fsm_next_phase(auto_phase));
    } else {
        --active_left;
        --red_left;
        setTimer(TIMER_TRAFFIC, SECOND_MS);
    }
    if (foreground) {
        fsm_automatic_resume();
    }
}

static void night_lights(uint8_t on) {
    night_lit = on;
    uint8_t color = on ? AMBER : OFF;
    traffic_set(LANE_1, color);
    traffic_set(LANE_2, color);
}

static void show_night_time(void) {
    char line[17];
    snprintf(line, sizeof(line), "NIGHT_MODE: %02us", (unsigned)night_seconds);
    lcd_show_line(1, line);
}

static void start_night_setup(void) {
    fsm_reset_button_repeats();
    status = NIGHT_CONFIG;
    night_seconds = NIGHT_DEFAULT_SEC;
    setTimer(TIMER_TRAFFIC, 0);
    night_lights(1);
    setTimer(TIMER_BLINK, NIGHT_SETUP_BLINK_MS);
    show_night_time();
    lcd_show_line(2, "3/4:+/- 2:OK");
}

static void end_night(void) {
    setTimer(TIMER_BLINK, 0);
    fsm_automatic_enter(AUTO_RED1_GREEN2);
    show_first_line();
}

static void run_night(const FsmButtons *buttons) {
    if (buttons->pressed[BTN_MODE] || buttons->held[BTN_MODE]) {
        end_night();
        return;
    }
    if (status == NIGHT_CONFIG) {
        if (buttons->pressed[BTN_SELECT]) {
            status = NIGHT_BLINK;
            night_lights(1);
            setTimer(TIMER_BLINK, NIGHT_ON_MS);
            setTimer(TIMER_TRAFFIC, SECOND_MS);
            lcd_show_line(2, "RUNNING (1:EXIT)");
            return;
        }
        int next = (int)night_seconds + fsm_adjustment(buttons);
        if (next >= 1 && next <= NIGHT_MAX_SEC && next != night_seconds) {
            night_seconds = (uint8_t)next;
            show_night_time();
        }
    } else if (isTimerEnd(TIMER_TRAFFIC)) {
        if (--night_seconds == 0) {
            end_night();
            return;
        }
        setTimer(TIMER_TRAFFIC, SECOND_MS);
        show_night_time();
    }
    if (isTimerEnd(TIMER_BLINK)) {
        night_lights(!night_lit);
        int duration = (status == NIGHT_CONFIG) ? NIGHT_SETUP_BLINK_MS :
                       (night_lit ? NIGHT_ON_MS : NIGHT_OFF_MS);
        setTimer(TIMER_BLINK, duration);
    }
}

void fsm_automatic_run(void) {
    if (status >= TUNING_INIT && status <= TUNING_MSG) {
        advance_clock(0);
        return;  /* Tuning owns the buttons and LCD row 2. */
    }
    if (status == INIT) {
        fsm_automatic_enter(AUTO_RED1_GREEN2);
        return;
    }
    if (status == NIGHT_INIT) {
        start_night_setup();
        return;
    }
    if (status == NIGHT_CONFIG || status == NIGHT_BLINK) {
        FsmButtons buttons;
        fsm_read_buttons(&buttons);
        run_night(&buttons);
        return;
    }
    if (status < AUTO_RED1_GREEN2 || status > AUTO_AMBER1_RED2) {
        return;
    }

    FsmButtons buttons;
    fsm_read_buttons(&buttons);
    if (buttons.held[BTN_MODE]) {
        start_night_setup();
    } else if (buttons.pressed[BTN_MODE]) {
        fsm_manual_enter(auto_phase->manual);
    } else {
        if (buttons.pressed[BTN_DEC]) {
            lcd_line1_mode = (lcd_line1_mode == LINE1_DHT20) ? LINE1_CONFIG : LINE1_DHT20;
            show_first_line();
        }
        advance_clock(1);
    }
}
