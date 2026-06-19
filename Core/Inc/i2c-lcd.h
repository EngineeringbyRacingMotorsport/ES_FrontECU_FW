/*
 * i2c-lcd.h
 *
 *  Created on: Apr 27, 2026
 *      Author: Elena
 */

#ifndef INC_I2C_LCD_H_
#define INC_I2C_LCD_H_

#include "stm32g4xx_hal.h"

void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_clear (void);
#endif /* INC_I2C_LCD_H_ */
