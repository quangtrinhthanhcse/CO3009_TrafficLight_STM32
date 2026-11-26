/*
 * dht20.h - Driver cảm biến nhiệt độ & độ ẩm DHT20 (hỗ trợ Non-blocking & Mock Data)
 */

#ifndef INC_DHT20_H_
#define INC_DHT20_H_

#include <stdint.h>
#include "main.h"

extern uint16_t value_x10[2]; // [0]: Độ ẩm x10, [1]: Nhiệt độ x10

void    dht20_init(void);     // Kiểm tra kết nối cảm biến (0x38), bật Mock nếu chưa cắm
void    dht20_update(void);   // Task chạy định kỳ qua Scheduler (non-blocking)
uint8_t dht20_is_mock(void);  // Trả về 1 nếu đang dùng Mock Data, 0 nếu dùng cảm biến thật

#endif /* INC_DHT20_H_ */
