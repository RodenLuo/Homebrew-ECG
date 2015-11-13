
/*
pin   description
1     VCC(3.3V)
2     GND
3     CE			//Pin2.3
4     RST			//Pin2.4
5     DC			//Pin2.2
6     DIN			//Pin2.1
7     CLK			//Pin2.0
8     Vlcd(+5V)

*/
#ifndef __Tnokia_5110_h_
#define __Tnokia_5110_h_

#include <msp430g2553.h>

#define   LCD_RST    BIT4
#define   LCD_CE     BIT3
#define   LCD_DC     BIT2
#define   LCD_DIN    BIT1
#define   LCD_CLK    BIT0

#define command     0
#define data        1

#define LCD_RST_0 	P2OUT &=~0x10
#define LCD_RST_1 	P2OUT |= 0x10
#define LCD_SCE_0 	P2OUT &=~0x08
#define LCD_SCE_1 	P2OUT |= 0x08
#define LCD_DC_0  	P2OUT &=~0x04
#define LCD_DC_1  	P2OUT |= 0x04
#define LCD_SDIN_0 	P2OUT &=~0x02
#define LCD_SDIN_1 	P2OUT |= 0x02
#define LCD_SCLK_0 	P2OUT &=~0x01
#define LCD_SCLK_1 	P2OUT |= 0x01

void Lcd_Init(void);
void Lcd_WriteByte(unsigned char value,unsigned char Type);
void Lcd_Clear(void);
void Lcd_Set_XY(unsigned char X,unsigned char Y);
void Lcd_WriteChar(unsigned char value);
void Lcd_Write_String(unsigned char X,unsigned char Y,char *s);
void Lcd_Write_Int(unsigned char X,unsigned char Y,int value);


#endif
