#include "global.h"

int status = INIT;

// Thời gian mặc định (giây): Đỏ 6s = Xanh 3s + Vàng 3s (A >= 3s, G>= A)
uint32_t time_red   = 6;
uint32_t time_green = 3;
uint32_t time_amber = 3;

uint32_t temp_time_red   = 6;
uint32_t temp_time_green = 3;

uint8_t lcd_line1_mode = LINE1_DHT20;

