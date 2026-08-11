
/** Put this in the src folder **/

#include "i2c-lcd.h"
extern I2C_HandleTypeDef hi2c2;  // change your handler here accordingly

#define SLAVE_ADDRESS_LCD 0x4E // change this according to ur setup

void lcd_send_cmd (char cmd)
{
  char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	HAL_I2C_Master_Transmit (&hi2c2, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1
	data_t[1] = data_u|0x09;  //en=0, rs=1
	data_t[2] = data_l|0x0D;  //en=1, rs=1
	data_t[3] = data_l|0x09;  //en=0, rs=1
	HAL_I2C_Master_Transmit (&hi2c2, SLAVE_ADDRESS_LCD,(uint8_t *) data_t, 4, 100);
}

void lcd_clear (void)
{
	lcd_send_cmd (0x00);
	for (int i=0; i<100; i++)
	{
		lcd_send_data (' ');
	}
}

/* Funció auxiliar per enviar només 4 bits (un nibble) */
static void lcd_send_cmd_half(char cmd)
{
    char data_u;
    uint8_t data_t[2];
    data_u = (cmd & 0xF0);
    data_t[0] = data_u | 0x0C; // en=1, rs=0, backlight=1
    data_t[1] = data_u | 0x08; // en=0, rs=0, backlight=1
    HAL_I2C_Master_Transmit(&hi2c2, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 2, 100);
}

void lcd_init(void)
{
    // Esperar >40ms des de la posada en marxa de la VCC (segons datasheet HD44780)
    HAL_Delay(50);

    // Secuència de Reset en mode 8-bits (enviant un sol nibble 0x30)
    lcd_send_cmd_half(0x30);
    HAL_Delay(5);
    lcd_send_cmd_half(0x30);
    HAL_Delay(1);
    lcd_send_cmd_half(0x30);
    HAL_Delay(10);

    // Canvi definitiu a mode 4 bits
    lcd_send_cmd_half(0x20);
    HAL_Delay(10);

    // A partir d'aquí ja es poden enviar 2 nibbles per comanda
    lcd_send_cmd(0x28); // 4-bit mode, 2/4 línies, matriu 5x8
    HAL_Delay(1);
    lcd_send_cmd(0x08); // Display OFF
    HAL_Delay(1);
    lcd_clear();        // Display Clear
    HAL_Delay(2);
    lcd_send_cmd(0x06); // Entry mode set (incrementa cursor)
    HAL_Delay(1);
    lcd_send_cmd(0x0C); // Display ON, Cursor OFF, Blink OFF
    HAL_Delay(1);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}
