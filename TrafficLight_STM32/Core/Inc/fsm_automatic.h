/*
 * fsm_automatic.h - Máy trạng thái Chế độ Tự động (Automatic Mode)
 */

#ifndef INC_FSM_AUTOMATIC_H_
#define INC_FSM_AUTOMATIC_H_

void fsm_automatic_run(void);
void fsm_automatic_enter(int target_status);
void fsm_automatic_start_bg(int target_status);
void fsm_automatic_resume(void);

#endif /* INC_FSM_AUTOMATIC_H_ */
