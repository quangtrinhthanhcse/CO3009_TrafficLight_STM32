/*
 * software_timer.h
 *
 *  Created on: Nov 19, 2026
 *      Author: trinhthanhquang
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#define MAX_TIMER 5
#define TIMER_TICK 10

// Phân bổ chỉ số timer dùng chung
#define TIMER_TRAFFIC    0   // Đếm lùi 1s cho pha Auto và bộ hẹn giờ Night Mode
#define TIMER_BLINK      1   // Nháy trường LCD ở Tuning và nháy đèn Vàng ở Night Mode
#define TIMER_MSG        2   // Giữ thông báo SAVED / ERR 3000 ms trên LCD

extern int timer_flag[MAX_TIMER];

void setTimer (int index, int duration); // duration (ms)
void timerRun (void);
int isTimerEnd (int index);


#endif /* INC_SOFTWARE_TIMER_H_ */
