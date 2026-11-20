/*
 * global.h - Khai báo các trạng thái FSM và biến thời gian dùng chung
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>

// Màu đèn dùng cho traffic_set(lane, color)
#define RED                  0
#define AMBER                1
#define GREEN                2
#define OFF                  3

// Trạng thái khởi tạo
#define INIT                 1

// Chế độ Tự động & Ban đêm (Night Mode)
#define AUTO_RED1_GREEN2     10
#define AUTO_RED1_AMBER2     11
#define AUTO_GREEN1_RED2     12
#define AUTO_AMBER1_RED2     13
#define NIGHT_INIT           14
#define NIGHT_CONFIG         15  // Đang hẹn giờ Night Mode (chờ BTN_2 xác nhận)
#define NIGHT_BLINK          16  // Đang nháy Vàng & đếm ngược về Auto

// Chế độ Bằng tay
#define MAN_INIT             20
#define MAN_RED1_GREEN2      21
#define MAN_RED1_AMBER2      22
#define MAN_GREEN1_RED2      23
#define MAN_AMBER1_RED2      24

// Chế độ Chỉnh thời gian
#define TUNING_INIT          30
#define TUNING_RED           31
#define TUNING_GREEN         32
#define TUNING_MSG           34  // Hiện thông báo SAVED / ERR trong 3s rồi về Auto

extern int status;

// Thời gian chạy chính thức (giây), luôn đảm bảo time_red = time_green + time_amber
extern uint32_t time_red;
extern uint32_t time_green;
extern uint32_t time_amber;

// Giá trị tạm khi đang chỉnh trong Tuning (Vàng tự tính A = R - G)
extern uint32_t temp_time_red;
extern uint32_t temp_time_green;

#endif /* INC_GLOBAL_H_ */
