#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "main.h"

#define NORMAL_STATE     GPIO_PIN_SET
#define PRESSED_STATE    GPIO_PIN_RESET
#define TIME_CYCLES      10   // Chu kỳ gọi hàm getKeyInput() (ms)
#define LONG_PRESS_TIME  100  // Thời gian nhận diện nhấn giữ: 100 * 10ms = 1000ms (1 giây)

typedef struct{
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t flag;              // Cờ báo sự kiện nhấn nhả thông thường (short press)
    uint8_t long_press_flag;   // Cờ báo sự kiện nhấn giữ (long press)
    uint8_t last_read;
    uint8_t last_read_stable;
    uint16_t tick;             // Biến đếm chống dội phím
    uint16_t long_press_tick;  // Biến đếm thời gian nhấn giữ
} button_Typedef;

extern button_Typedef btn[5];

void button_init(void);
void getKeyInput(void);
uint8_t isButtonPressed(uint8_t index);
uint8_t isButtonLongPressed(uint8_t index);

#endif /* INC_BUTTON_H_ */
