/*
 * button.h
 *
 *  Created on: Nov 19, 2026
 *      Author:
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#define NUM_BUTTONS 4

#define BTN_MODE 	0 	// BTN_1 (PA8 / D7): Nhấn ngắn đổi chế độ; nhấn giữ (>=1s) bật/tắt Night Mode
#define BTN_SELECT 	1	// BTN_2 (PA9 / D8): Chọn đèn / chuyển pha
#define BTN_INC 	2	// BTN_3 (PC7 / D9): Tăng thời gian (hoặc về Auto từ Manual)
#define BTN_DEC 	3	// BTN_4 (PB6 / D10): Giảm thời gian

void button_init(void);
void getInput(void);
int isPressed(int index);
int isLongPressed(int index);


#endif /* INC_BUTTON_H_ */
