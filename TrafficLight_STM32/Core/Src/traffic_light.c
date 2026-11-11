#include "traffic_light.h"

// Hàm phụ trợ nội bộ để điều khiển nhanh 2 chân tín hiệu của một tuyến
static void set_lane_pins(GPIO_TypeDef* port_a, uint16_t pin_a, GPIO_TypeDef* port_b, uint16_t pin_b, uint8_t state) {
    // state: 0=OFF, 1=RED, 2=GREEN, 3=YELLOW (dựa theo bảng tổ hợp phần cứng 3.2)
    switch (state) {
        case 0: // Tắt (0,0)
            HAL_GPIO_WritePin(port_a, pin_a, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(port_b, pin_b, GPIO_PIN_RESET);
            break;
        case 1: // Đỏ (0,1) - Tùy thuộc khảo sát thực tế phần cứng của nhóm
            HAL_GPIO_WritePin(port_a, pin_a, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(port_b, pin_b, GPIO_PIN_SET);
            break;
        case 2: // Xanh (1,0)
            HAL_GPIO_WritePin(port_a, pin_a, GPIO_PIN_SET);
            HAL_GPIO_WritePin(port_b, pin_b, GPIO_PIN_RESET);
            break;
        case 3: // Vàng (1,1)
            HAL_GPIO_WritePin(port_a, pin_a, GPIO_PIN_SET);
            HAL_GPIO_WritePin(port_b, pin_b, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}

void traffic_init(void) {
    // Khởi tạo ban đầu tắt toàn bộ đèn
    traffic_light(TRAFFIC_OFF);
}

void traffic_light(uint8_t status) {
    switch (status) {
        case TRAFFIC_RED_GREEN:
            // Tuyến 1: Đỏ (1), Tuyến 2: Xanh (2)
            set_lane_pins(TL1_A_GPIO_Port, TL1_A_Pin, TL1_B_GPIO_Port, TL1_B_Pin, 1);
            set_lane_pins(TL2_A_GPIO_Port, TL2_A_Pin, TL2_B_GPIO_Port, TL2_B_Pin, 2);
            break;

        case TRAFFIC_RED_YELLOW:
            // Tuyến 1: Đỏ (1), Tuyến 2: Vàng (3)
            set_lane_pins(TL1_A_GPIO_Port, TL1_A_Pin, TL1_B_GPIO_Port, TL1_B_Pin, 1);
            set_lane_pins(TL2_A_GPIO_Port, TL2_A_Pin, TL2_B_GPIO_Port, TL2_B_Pin, 3);
            break;

        case TRAFFIC_GREEN_RED:
            // Tuyến 1: Xanh (2), Tuyến 2: Đỏ (1)
            set_lane_pins(TL1_A_GPIO_Port, TL1_A_Pin, TL1_B_GPIO_Port, TL1_B_Pin, 2);
            set_lane_pins(TL2_A_GPIO_Port, TL2_A_Pin, TL2_B_GPIO_Port, TL2_B_Pin, 1);
            break;

        case TRAFFIC_YELLOW_RED:
            // Tuyến 1: Vàng (3), Tuyến 2: Đỏ (1)
            set_lane_pins(TL1_A_GPIO_Port, TL1_A_Pin, TL1_B_GPIO_Port, TL1_B_Pin, 3);
            set_lane_pins(TL2_A_GPIO_Port, TL2_A_Pin, TL2_B_GPIO_Port, TL2_B_Pin, 1);
            break;

        case TRAFFIC_RED_RED:
            // Cả hai tuyến đều Đỏ (1)
            set_lane_pins(TL1_A_GPIO_Port, TL1_A_Pin, TL1_B_GPIO_Port, TL1_B_Pin, 1);
            set_lane_pins(TL2_A_GPIO_Port, TL2_A_Pin, TL2_B_GPIO_Port, TL2_B_Pin, 1);
            break;

        case TRAFFIC_OFF:
        default:
            // Tắt toàn bộ đèn
            set_lane_pins(TL1_A_GPIO_Port, TL1_A_Pin, TL1_B_GPIO_Port, TL1_B_Pin, 0);
            set_lane_pins(TL2_A_GPIO_Port, TL2_A_Pin, TL2_B_GPIO_Port, TL2_B_Pin, 0);
            break;
    }
}
