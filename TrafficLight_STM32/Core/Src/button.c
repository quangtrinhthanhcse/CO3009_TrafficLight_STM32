/*
 * button.c
 *
 *  Created on: Nov 19, 2026
 *      Author: Home
 */

#include "button.h"
#include "main.h"

#define NORMAL_STATE		GPIO_PIN_SET
#define PRESSED_STATE		GPIO_PIN_RESET
#define LONG_PRESSED_TICK	100
#define AUTO_REPEAT_TICKS	20

static GPIO_TypeDef* BTN_PORT[NUM_BUTTONS] = {
    BTN_1_GPIO_Port, BTN_2_GPIO_Port, BTN_3_GPIO_Port, BTN_4_GPIO_Port
};
static uint16_t BTN_PIN[NUM_BUTTONS] = {
    BTN_1_Pin, BTN_2_Pin, BTN_3_Pin, BTN_4_Pin
};

static int KeyReg0[NUM_BUTTONS];
static int KeyReg1[NUM_BUTTONS];
static int KeyReg2[NUM_BUTTONS];
static int KeyReg3[NUM_BUTTONS];

static int TimerForKeyPress[NUM_BUTTONS];
static int button_flag[NUM_BUTTONS];
static int button_long_flag[NUM_BUTTONS];
static int long_pressed_handled[NUM_BUTTONS];

void button_init (void){
	for (int i = 0; i < NUM_BUTTONS; i++){
		KeyReg0[i] = KeyReg1[i] = KeyReg2[i] = KeyReg3[i] = NORMAL_STATE;
		TimerForKeyPress[i] = LONG_PRESSED_TICK;
		button_flag[i] = 0;
		button_long_flag[i] = 0;
		long_pressed_handled[i] = 0;
	}
}

void getInput (void){
	for (int i = 0; i < NUM_BUTTONS; i++){
		KeyReg0[i] = KeyReg1[i];
		KeyReg1[i] = KeyReg2[i];
		KeyReg2[i] = HAL_GPIO_ReadPin(BTN_PORT[i], BTN_PIN[i]);
		if ( (KeyReg0[i] == KeyReg1[i]) && (KeyReg1[i] == KeyReg2[i])){
			if (KeyReg3[i] != KeyReg2[i]){
				KeyReg3[i] = KeyReg2[i];
				if (KeyReg2[i] == PRESSED_STATE) {
					TimerForKeyPress[i] = LONG_PRESSED_TICK;
					long_pressed_handled[i] = 0;
					/* Các nút khác phản hồi ngay khi nhấn xuống; riêng BTN_MODE chờ nhả để phân biệt nhấn giữ */
					if (i != BTN_MODE) {
						button_flag[i] = 1;
					}
				} else {
					if (i == BTN_MODE && long_pressed_handled[i] == 0) {
						button_flag[i] = 1;
					}
				}
			}
			else{
				if (KeyReg2[i] == PRESSED_STATE){
					if (i == BTN_MODE) {
						if (long_pressed_handled[i] == 0) {
							TimerForKeyPress[i]--;
							if (TimerForKeyPress[i] <= 0){
								button_long_flag[i] = 1;
								long_pressed_handled[i] = 1;
							}
						}
					} else {
						TimerForKeyPress[i]--;
						if (TimerForKeyPress[i] <= 0){
							button_long_flag[i] = 1;
							TimerForKeyPress[i] = AUTO_REPEAT_TICKS;
						}
					}
				}
			}
		}
	}
}

int isPressed (int index){
	if (index < 0 || index >= NUM_BUTTONS) return 0;
	if (button_flag[index] == 1) {
		button_flag[index] = 0;
		return 1;
	}
	else return 0;
}

int isLongPressed (int index){
	if (index < 0 || index >= NUM_BUTTONS) return 0;
	if (button_long_flag[index] == 1) {
		button_long_flag[index] = 0;
		return 1;
	}
	else return 0;


}
