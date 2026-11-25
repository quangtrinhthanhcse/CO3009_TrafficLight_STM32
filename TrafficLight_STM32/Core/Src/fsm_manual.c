#include "fsm_manual.h"
#include "fsm_automatic.h"
#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"
#include "i2c-lcd.h"
#include <stddef.h>

void fsm_manual_enter(int target_status) {
    const FsmPhase *phase = fsm_find_phase(target_status);
    if (phase == NULL) {
        return;
    }
    setTimer(TIMER_TRAFFIC, 0);
    fsm_apply_phase(phase);
    status = phase->manual;
    if (phase->active_lane == LANE_1) {
        lcd_show_line(2, phase->color == GREEN ? "M 1:GREEN 2:RED" : "M 1:AMBER 2:RED");
    } else {
        lcd_show_line(2, phase->color == GREEN ? "M 1:RED 2:GREEN" : "M 1:RED 2:AMBER");
    }
}

void fsm_manual_run(void) {
    if (status == MAN_INIT) {
        fsm_manual_enter(MAN_RED1_GREEN2);
        return;
    }
    if (status < MAN_RED1_GREEN2 || status > MAN_AMBER1_RED2) {
        return;
    }
    const FsmPhase *phase = fsm_find_phase(status);
    FsmButtons buttons;
    fsm_read_buttons(&buttons);
    if (buttons.pressed[BTN_MODE]) {
        fsm_automatic_start_bg(phase->automatic);
        status = TUNING_INIT;
    } else if (buttons.pressed[BTN_INC]) {
        fsm_automatic_enter(fsm_next_phase(phase)->automatic);
    } else if (buttons.pressed[BTN_SELECT]) {
        fsm_manual_enter(fsm_next_phase(phase)->manual);
    }
}
