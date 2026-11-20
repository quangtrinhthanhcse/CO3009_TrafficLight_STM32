/*
 * software_timer.c
 *
 *  Created on: Nov 19, 2026
 *      Author: quangtrinhthanh
 */
#include "software_timer.h"

int timer_counter[MAX_TIMER] = {0};
int timer_flag[MAX_TIMER] = {0};

void setTimer (int index, int duration){
	if (index < 0 || index >= MAX_TIMER)	return;

	timer_counter[index] = duration/ TIMER_TICK;
	if (duration > 0 && timer_counter[index] == 0){
		timer_counter[index] = 1;
	}
	timer_flag[index] = 0;
}

void timerRun (void){
	for (int i = 0; i < MAX_TIMER; i++){
		if (timer_counter[i] > 0){
			timer_counter[i]--;
			if (timer_counter[i] <= 0) timer_flag[i] = 1;
		}
	}
}

int isTimerEnd (int index){
	if (index < 0 || index >= MAX_TIMER) return 0;
	if (timer_flag[index] == 1){
		timer_flag[index] = 0;
		return 1;
	}
	return 0;
}
