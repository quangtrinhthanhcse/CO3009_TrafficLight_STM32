/** 
  ******************************************************************************
  * @file           : i2c-lcd.h
  * @brief          : Header for PCF8574 16x2 I2C LCD (Original SourceCode)
  ******************************************************************************
  */

#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_

#include "stm32f1xx_hal.h"

#define SLAVE_ADDRESS_LCD 0x4E // Địa chỉ 8-bit mặc định của PCF8574

void lcd_init (void);   // initialize lcd
void lcd_send_cmd (char cmd);  // send command to the lcd
void lcd_send_data (char data);  // send data to the lcd
void lcd_send_string (char *str);  // send string to the lcd
void lcd_clear_display (void);	//clear display lcd
void lcd_goto_XY (int row, int col); //set proper location on screen
void lcd_set_address (uint8_t addr); //set dynamic I2C address

// Hiển thị theo dòng cho ứng dụng đèn giao thông
void lcd_show_line (uint8_t row, const char *str);
void lcd_display_env (uint16_t temp_x10, uint16_t hum_x10, uint8_t is_mock);

void lcd_display_config (uint32_t red, uint32_t green, uint32_t amber );

#endif /* INC_I2C_LCD_H_ */
