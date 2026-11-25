#ifndef INC_FSM_AUTOMATIC_H_
#define INC_FSM_AUTOMATIC_H_

#include <stdint.h>
#include "button.h"

void fsm_automatic_run(void);
void fsm_automatic_enter(int target_status);
void fsm_automatic_start_bg(int target_status);
void fsm_automatic_resume(void);

/* Shared by the three FSM tasks. Only the mode that owns the buttons reads them. */
typedef struct {
    uint8_t pressed[NUM_BUTTONS];
    uint8_t held[NUM_BUTTONS];
} FsmButtons;

void fsm_reset_button_repeats(void);
void fsm_read_buttons(FsmButtons *buttons);
int fsm_adjustment(const FsmButtons *buttons);

/* Auto and Manual follow the same four phases. The opposite lane is always red. */
typedef struct {
    int automatic;
    int manual;
    uint8_t active_lane;
    uint8_t color;
} FsmPhase;

const FsmPhase *fsm_find_phase(int status_code);
const FsmPhase *fsm_next_phase(const FsmPhase *phase);
void fsm_apply_phase(const FsmPhase *phase);

#endif
