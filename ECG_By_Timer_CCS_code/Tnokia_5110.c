
#include "Tnokia_5110.h"
#include "english_6x8_pixel.h"

/*******************************************************
Function:
Input:
output:
description:
*******************************************************/
void Lcd_Init(void)
{
	P2DIR |= LCD_RST;	//set output
	P2DIR |= LCD_CE;	//set output
	P2DIR |= LCD_DC;	//set output
	P2DIR |= LCD_DIN;	//set output
	P2DIR |= LCD_CLK;	//set output

	LCD_RST_0;
	_NOP();				//delay >1us
	LCD_RST_1;

	LCD_SCE_0;
	_NOP();				//delay >1us
	LCD_SCE_1;
	_NOP();				//delay >1us

	Lcd_WriteByte(0x21, command);	//PD=0,V=0,H=1
	Lcd_WriteByte(0xc8, command);	//set Vop
	Lcd_WriteByte(0x06, command);	//set TCx
	Lcd_WriteByte(0x13, command);	//set BSx
	Lcd_WriteByte(0x20, command);	//PD=0,V=0,H=0
	Lcd_Clear();
	Lcd_WriteByte(0x0c, command);	//set display configuration

	LCD_SCE_0;
}

/*******************************************************
Function:
Input:
output:
description:
*******************************************************/
void Lcd_Clear(void)
{
	unsigned int i;

	Lcd_WriteByte(0x0c,command);
	Lcd_WriteByte(0x80,command);

	for(i=0; i<504; i++)		//fill all Zero value
		Lcd_WriteByte(0,data);

}
/*******************************************************
Function:
Input:
output:
description:
*******************************************************/
void Lcd_WriteByte(unsigned char value,unsigned char Type)
{
	unsigned char i;

	LCD_SCE_0;					//SCE=0

	if(Type==0)					//set DC
		LCD_DC_0;
	else
		LCD_DC_1;

	for(i=0;i<8;i++)
	{
		if(value & 0x80)
			LCD_SDIN_1;
		else
			LCD_SDIN_0;

		LCD_SCLK_0;
		value = value << 1;
		LCD_SCLK_1;

	}
	LCD_SCE_1;					//SCE=1

}
/*******************************************************
Function:
Input:	0<=X<=83	0<=Y<=5
output:
description:
*******************************************************/
void Lcd_Set_XY(unsigned char X,unsigned char Y)
{
	Lcd_WriteByte(0x40 | Y, command);	//set Y Addr
	Lcd_WriteByte(0x80 | X, command);	//set X addr
}
/*******************************************************
Function:
Input:
output:
description:
*******************************************************/
void Lcd_WriteChar(unsigned char value)
{
	unsigned char line;

	value = value-0x20;			//space ascii=0x20
								//'0'   ascii=0x30
	for(line=0;line<6;line++)
		Lcd_WriteByte(font6x8[value][line],data);
}

/*******************************************************
Function:
Input:
output:
description:
*******************************************************/
void Lcd_Write_String(unsigned char X,unsigned char Y,char *s)
{
	Lcd_Set_XY(X,Y);			//set Addr

	while(*s)
	{
		Lcd_WriteChar(*s);
		s++;
	}
}

/*******************************************************
Function:
Input:
output:
description:
*******************************************************/
void Lcd_Write_Int(unsigned char X,unsigned char Y,int value)
{
	Lcd_Set_XY(X,Y);			//set Addr
//	int a0 = value / 100;
//	value = value % 100;
	int a1 = value / 10;
	int a2 = value % 10;
//	a0 = a0 + '0';
	a1 = a1 + '0';
	a2 = a2 + '0';
//	Lcd_WriteChar(a0);
	Lcd_WriteChar(a1);
	Lcd_WriteChar(a2);
}









