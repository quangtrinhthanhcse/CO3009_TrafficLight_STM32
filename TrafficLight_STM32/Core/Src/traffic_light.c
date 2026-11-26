/*
 * traffic_light.c - Hiện thực driver điều khiển đèn giao thông 2 tuyến
 */

#include "traffic_light.h"
#include "main.h"

void traffic_init(void) {
	traffic_set(LANE_1, OFF);
	traffic_set(LANE_2, OFF);
}

void traffic_set(uint8_t lane, uint8_t color) {
	GPIO_PinState pin_a = GPIO_PIN_RESET;
	GPIO_PinState pin_b = GPIO_PIN_RESET;

	switch (color){
	case OFF:
		pin_a = GPIO_PIN_RESET;
		pin_b = GPIO_PIN_RESET;
		break;

	case AMBER:
		pin_a = GPIO_PIN_RESET;
		pin_b = GPIO_PIN_SET;
		break;

	case GREEN:
		pin_a = GPIO_PIN_SET;
		pin_b = GPIO_PIN_RESET;
		break;

	case RED:
		pin_a = GPIO_PIN_SET;
		pin_b = GPIO_PIN_SET;
		break;

	default: return;
	}

	if (lane == LANE_1 ){
			HAL_GPIO_WritePin(TL_1A_GPIO_Port, TL_1A_Pin, pin_a);
			HAL_GPIO_WritePin(TL_1B_GPIO_Port, TL_1B_Pin, pin_b);
		}

	else if (lane == LANE_2 ){
			HAL_GPIO_WritePin(TL_2A_GPIO_Port, TL_2A_Pin, pin_a);
			HAL_GPIO_WritePin(TL_2B_GPIO_Port, TL_2B_Pin, pin_b);
		}

}
