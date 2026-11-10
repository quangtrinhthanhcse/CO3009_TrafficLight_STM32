/*
 * global.c - Khởi tạo giá trị ban đầu cho các biến toàn cục
 */

#include "global.h"

// Trạng thái ban đầu khi khởi động hệ thống
int status = INIT;

// Thời gian cài đặt mặc định: Đỏ 5s, Xanh 3s, Vàng 2s
uint32_t time_red   = 5;
uint32_t time_green = 3;
uint32_t time_amber = 2;

// Khởi tạo các biến tạm thời gian dùng cho Tuning Mode
uint32_t temp_time_red   = 5;
uint32_t temp_time_green = 3;
uint32_t temp_time_amber = 2;

// Khởi tạo thông số môi trường ban đầu
float temperature = 0.0f;
float humidity    = 0.0f;
