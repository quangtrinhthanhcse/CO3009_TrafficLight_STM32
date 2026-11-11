#include "button.h"

// Khởi tạo danh sách 5 nút nhấn dựa theo định nghĩa trong main.h (BTN_0 đến BTN_4)
button_Typedef btn[5] = {
    {BTN_0_GPIO_Port, BTN_0_Pin, 0, 0, NORMAL_STATE, NORMAL_STATE, 0, 0},
    {BTN_1_GPIO_Port, BTN_1_Pin, 0, 0, NORMAL_STATE, NORMAL_STATE, 0, 0},
    {BTN_2_GPIO_Port, BTN_2_Pin, 0, 0, NORMAL_STATE, NORMAL_STATE, 0, 0},
    {BTN_3_GPIO_Port, BTN_3_Pin, 0, 0, NORMAL_STATE, NORMAL_STATE, 0, 0},
    {BTN_4_GPIO_Port, BTN_4_Pin, 0, 0, NORMAL_STATE, NORMAL_STATE, 0, 0}
};

void button_init(void) {
    // Khởi tạo ngoại vi nút nhấn nếu cần
}

// Hàm kiểm tra nhấn nhả thông thường
uint8_t isButtonPressed(uint8_t index){
    if (index >= sizeof(btn) / sizeof(button_Typedef)) return 0;
    if (btn[index].flag == 0) return 0;
    btn[index].flag = 0; // Đọc xong tự động xóa cờ
    return 1;
}

// Hàm kiểm tra sự kiện nhấn giữ
uint8_t isButtonLongPressed(uint8_t index){
    if (index >= sizeof(btn) / sizeof(button_Typedef)) return 0;
    if (btn[index].long_press_flag == 0) return 0;
    btn[index].long_press_flag = 0; // Đọc xong tự động xóa cờ
    return 1;
}

// Hàm quét trạng thái phím (gọi định kỳ mỗi 10ms trong Scheduler)
void getKeyInput(){
    for(uint8_t i = 0; i < sizeof(btn) / sizeof(button_Typedef); i++){
        uint8_t current_btn = HAL_GPIO_ReadPin(btn[i].port, btn[i].pin);

        // Chống dội phím (Debounce)
        if (btn[i].last_read != current_btn) {
            btn[i].tick = 0;
        } else {
            btn[i].tick += TIME_CYCLES;
        }

        if (btn[i].tick >= 30){ // Ổn định sau 30ms
            if (current_btn == PRESSED_STATE) {
                if (btn[i].last_read_stable == NORMAL_STATE) {
                    // Vừa mới nhấn xuống -> reset bộ đếm giữ phím
                    btn[i].long_press_tick = 0;
                } else {
                    // Đang giữ phím liên tục -> tăng bộ đếm thời gian giữ
                    btn[i].long_press_tick += TIME_CYCLES;
                    if (btn[i].long_press_tick >= LONG_PRESS_TIME) {
                        if (btn[i].long_press_flag == 0) {
                            btn[i].long_press_flag = 1; // Kích hoạt cờ nhấn giữ khi đủ thời gian (1 giây)
                        }
                    }
                }
            } else {
                // Trạng thái nhả phím (NORMAL_STATE)
                if (btn[i].last_read_stable == PRESSED_STATE) {
                    // Vừa mới nhả phím ra: nếu thời gian đè chưa đạt ngưỡng long press thì tính là nhấn nhả ngắn
                    if (btn[i].long_press_tick < LONG_PRESS_TIME) {
                        btn[i].flag = 1;
                    }
                }
                // Reset lại các biến đếm khi thả phím hoàn toàn
                btn[i].long_press_tick = 0;
            }
            btn[i].last_read_stable = current_btn;
        }
        btn[i].last_read = current_btn;
    }
}
