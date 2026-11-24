/*
 * dht20.c - Driver DHT20 (Tự động chuyển Mock Data khi thiếu cảm biến vật lý)
 */

#include "dht20.h"

extern I2C_HandleTypeDef hi2c1;

#define SLAVE_ADDRESS_DHT20 (0x38 << 1)

uint16_t value_x10[2] = {650, 285}; // Mặc định: 65.0% và 28.5°C

static uint8_t dht20_connected = 0; // 1 = có cảm biến thật, 0 = dùng Mock Data
static uint8_t dht20_state = 0;     // 0 = chờ gửi lệnh đo, 1 = chờ đọc kết quả (sau >= 80ms)

void dht20_init(void) {
    if (HAL_I2C_IsDeviceReady(&hi2c1, SLAVE_ADDRESS_DHT20, 2, 10) == HAL_OK) {
        dht20_connected = 1;

        // Gửi lệnh khởi tạo thanh ghi theo datasheet DHT20
        uint8_t init_cmd[3] = {0xBE, 0x08, 0x00};
        HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_DHT20, init_cmd, 3, 100);
    } else {
        dht20_connected = 0;
        value_x10[0] = 650;
        value_x10[1] = 285;
    }
    dht20_state = 0;
}

void dht20_update(void) {
    if (!dht20_connected) {
        // Mock Mode: dao động nhẹ nhiệt độ (28.0..29.0°C) và độ ẩm (64.0..66.0%)
        static int8_t mock_step = 0;
        static int8_t mock_dir  = 1;

        mock_step += mock_dir;
        if (mock_step >= 5) {
            mock_dir = -1;
        } else if (mock_step <= -5) {
            mock_dir = 1;
        }

        value_x10[1] = (uint16_t)(285 + mock_step);
        value_x10[0] = (uint16_t)(650 - mock_step * 2);
        return;
    }

    // Real Sensor: Non-blocking 2 pha qua Scheduler (chu kỳ gọi >= 100ms)
    if (dht20_state == 0) {
        uint8_t trigger_cmd[3] = {0xAC, 0x33, 0x00};
        if (HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_DHT20, trigger_cmd, 3, 50) == HAL_OK) {
            dht20_state = 1;
        }
    } else {
        uint8_t data[7] = {0};
        if (HAL_I2C_Master_Receive(&hi2c1, SLAVE_ADDRESS_DHT20 | 0x01, data, 7, 50) == HAL_OK) {
            // Bit 7 của byte status == 0 nghĩa là cảm biến đã đo xong
            if ((data[0] & 0x80) == 0) {
                uint32_t humid_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | (data[3] >> 4);
                uint32_t temp_raw  = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | data[5];

                value_x10[0] = (uint16_t)((humid_raw * 1000UL) >> 20);
                int32_t t_x10 = (int32_t)((temp_raw * 2000UL) >> 20) - 500;
                value_x10[1] = (t_x10 < 0) ? 0 : (uint16_t)t_x10;
            }
        }
        dht20_state = 0;
    }
}

uint8_t dht20_is_mock(void) {
    return (dht20_connected == 0) ? 1 : 0;
}
