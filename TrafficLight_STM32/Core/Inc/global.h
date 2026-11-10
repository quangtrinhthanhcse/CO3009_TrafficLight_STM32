/*
 * global.h - Khai báo các trạng thái và biến toàn cục dùng chung cho BTL CO3009
 * Thành viên phụ trách: TV1 (chủ trì) & TV3 (phối hợp)
 */

#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>

/* ==================================================================== */
/* 1. MÀU ĐÈN TỪNG TUYẾN (Dùng làm tham số cho hàm traffic_set(lane, color)) */
/* ==================================================================== */
#define RED                  0
#define AMBER                1   // Đèn vàng
#define GREEN                2
#define OFF                  3

/* ==================================================================== */
/* 2. CÁC TRẠNG THÁI MÁY TRẠNG THÁI (FSM STATUS)                         */
/* ==================================================================== */

// Trạng thái khởi động
#define INIT                 1

// Chế độ 1: Tự động (Automatic Mode) - Luân phiên 4 pha đèn 2 tuyến
#define AUTO_RED1_GREEN2     10
#define AUTO_RED1_AMBER2     11
#define AUTO_GREEN1_RED2     12
#define AUTO_AMBER1_RED2     13

// Chế độ 2: Bằng tay (Manual Mode) - Chuyển pha an toàn qua nút bấm
#define MAN_INIT             20
#define MAN_RED1_GREEN2      21
#define MAN_RED1_AMBER2      22
#define MAN_GREEN1_RED2      23
#define MAN_AMBER1_RED2      24

// Chế độ 3: Chỉnh thời gian (Tuning Mode) - Menu chỉnh thông số trên LCD
#define TUNING_INIT          30
#define TUNING_RED           31  // Đang chỉnh thời gian đèn Đỏ
#define TUNING_GREEN         32  // Đang chỉnh thời gian đèn Xanh
#define TUNING_AMBER         33  // Đang chỉnh thời gian đèn Vàng

/* ==================================================================== */
/* 3. CÁC BIẾN TOÀN CỤC DÙNG CHUNG TRONG TOÀN HỆ THỐNG                  */
/* ==================================================================== */

// Biến trạng thái máy FSM hiện tại (được gọi trong main, fsm_*, scheduler)
extern int status;

// Thời gian hoạt động chính thức của các đèn (đơn vị: giây)
// Mặc định thỏa ràng buộc: time_red = time_green + time_amber (5 = 3 + 2)
extern uint32_t time_red;
extern uint32_t time_green;
extern uint32_t time_amber;

// Biến tạm lưu giá trị thời gian khi đang chỉnh trong Chế độ Tuning
extern uint32_t temp_time_red;
extern uint32_t temp_time_green;
extern uint32_t temp_time_amber;

// Dữ liệu đọc từ cảm biến DHT20 để LCD hiển thị
extern float temperature;
extern float humidity;

#endif /* INC_GLOBAL_H_ */
