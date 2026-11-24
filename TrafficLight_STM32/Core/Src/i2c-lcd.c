/** 
  ******************************************************************************
  * @file           : i2c-lcd.c
  * @brief          : Driver for PCF8574 16x2 I2C LCD (Original SourceCode)
  ******************************************************************************
  */

#include "i2c-lcd.h"
#include <stdio.h>
#include <stdint.h>

extern I2C_HandleTypeDef hi2c1;

// Địa chỉ 8-bit mặc định của PCF8574 (0x4E)
static uint8_t lcd_slave_addr = SLAVE_ADDRESS_LCD;

void lcd_set_address(uint8_t addr)
{
	lcd_slave_addr = addr;
}

void lcd_send_cmd (char cmd)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);
	data_t[0] = data_u | 0x0C;  // en=1, rs=0, backlight=1
	data_t[1] = data_u | 0x08;  // en=0, rs=0, backlight=1
	data_t[2] = data_l | 0x0C;  // en=1, rs=0, backlight=1
	data_t[3] = data_l | 0x08;  // en=0, rs=0, backlight=1
	HAL_I2C_Master_Transmit (&hi2c1, lcd_slave_addr, (uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D;  // en=1, rs=1, backlight=1
	data_t[1] = data_u | 0x09;  // en=0, rs=1, backlight=1
	data_t[2] = data_l | 0x0D;  // en=1, rs=1, backlight=1
	data_t[3] = data_l | 0x09;  // en=0, rs=1, backlight=1
	HAL_I2C_Master_Transmit (&hi2c1, lcd_slave_addr, (uint8_t *) data_t, 4, 100);
}

void lcd_init (void)
{
	HAL_Delay(50);
	lcd_send_cmd (0x33); /* set 4-bits interface */
	lcd_send_cmd (0x32);
	HAL_Delay(50);
	lcd_send_cmd (0x28); /* start to set LCD function */
	HAL_Delay(50);
	lcd_send_cmd (0x01); /* clear display */
	HAL_Delay(50);
	lcd_send_cmd (0x06); /* set entry mode */
	HAL_Delay(50);
	lcd_send_cmd (0x0C); /* set display to on, cursor off, blink off */	
	HAL_Delay(50);
	lcd_send_cmd (0x02); /* move cursor to home and set data address to 0 */
	HAL_Delay(50);
	lcd_send_cmd (0x80);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}

void lcd_clear_display (void)
{
	lcd_send_cmd (0x01); // clear display
}

void lcd_goto_XY (int row, int col)
{
	uint8_t pos_Addr;
	if (row == 1) 
	{
		pos_Addr = 0x80 + row - 1 + col;
	}
	else
	{
		pos_Addr = 0x80 | (0x40 + col);
	}
	lcd_send_cmd(pos_Addr);
}

void lcd_show_line (uint8_t row, const char *str)
{
	char buf[17];
	snprintf(buf, sizeof(buf), "%-16.16s", (str != NULL) ? str : "");
	lcd_goto_XY(row, 0);
	lcd_send_string(buf);
}

void lcd_display_env (uint16_t temp_x10, uint16_t hum_x10, uint8_t is_mock)
{
	char buf[17];
	snprintf(buf, sizeof(buf), "T:%u.%uC H:%u.%u%%%c",
			 temp_x10 / 10, temp_x10 % 10,
			 hum_x10 / 10, hum_x10 % 10,
			 is_mock ? 'M' : ' ');
	lcd_show_line(1, buf);
}

void lcd_display_config (uint32_t red, uint32_t green, uint32_t amber){
	char buf[17];
	snprintf(buf, sizeof(buf), "R:%02lu  G:%02lu  A:%02lu",
	             (unsigned long)red, (unsigned long)green, (unsigned long)amber);
	lcd_show_line(1, buf);
}
