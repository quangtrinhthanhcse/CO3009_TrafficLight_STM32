#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include "main.h"

// Định nghĩa các trạng thái kết hợp cho cả 2 tuyến đèn giao thông
#define TRAFFIC_OFF           0
#define TRAFFIC_RED_GREEN     1  // Tuyến 1 Đỏ - Tuyến 2 Xanh
#define TRAFFIC_RED_YELLOW    2  // Tuyến 1 Đỏ - Tuyến 2 Vàng
#define TRAFFIC_GREEN_RED     3  // Tuyến 1 Xanh - Tuyến 2 Đỏ
#define TRAFFIC_YELLOW_RED    4  // Tuyến 1 Vàng - Tuyến 2 Đỏ
#define TRAFFIC_RED_RED       5  // Cả 2 tuyến Đỏ
#define TRAFFIC_YELLOW_YELLOW 6  // Cả 2 tuyến Vàng
#define TRAFFIC_GREEN_GREEN   7  // Cả 2 tuyến Xanh

void traffic_init(void);
void traffic_light(uint8_t status);

#endif /* INC_TRAFFIC_LIGHT_H_ */
