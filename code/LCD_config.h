#include<stdio.h>
#include<stdint.h>
#include"GPIO.h"

#define commandport GPIOA->ODR
#define rs 8
#define e 9
#define dataport GPIOB->ODR
#define eset() GPIOA->BSRR|=(1<<e)
#define eclear() GPIOA->BRR|=(1<<e)

void SysTick_Init()
{
	SysTick->CTRL=0;
	SysTick->LOAD=799; // 100us
	SysTick->VAL=0;
	SysTick->CTRL=0x05;
}
void delay_us(uint32_t time)//100us*time
{
	SysTick->VAL=0;
	while(time>0)
	{
		if(SysTick->CTRL&0x010000)
			time--;
	}
}
void lcd_pulse()
{
	eset();
	delay_us(2);
	eclear();
	delay_us(2);
}
void lcd_sendcommand(uint32_t cmd)
{
	commandport&=~(0x01<<rs);
	dataport=(dataport&~0xF000)|((cmd&0xF0)<<8);
	lcd_pulse();//400us
	dataport=(dataport&~0xF000)|((cmd&0x0F)<<12);
	lcd_pulse();//400us
	commandport|=(0x01<<rs);
}
void lcd_senddata(uint32_t data)
{
	commandport|=(0x01<<rs);
	dataport=(dataport&~0xF000)|((data&0xF0)<<8);
	lcd_pulse();
	dataport=(dataport&~0xF000)|((data&0x0F)<<12);
	lcd_pulse();
}
void lcd_clear()
{
	lcd_sendcommand(0x01);
	delay_us(20);
}
void lcd_initialize()
{
	delay_us(400);
	lcd_sendcommand(0x30);
	delay_us(45);
	lcd_sendcommand(0x30);
	delay_us(2);
	lcd_sendcommand(0x30);
	delay_us(2);
	lcd_sendcommand(0x20);
	delay_us(1);
	lcd_sendcommand(0x20);
	delay_us(1);
	lcd_sendcommand(0x08);
	delay_us(1);
	lcd_sendcommand(0x01);
	delay_us(20);
	lcd_sendcommand(0x06);
	delay_us(1);
	lcd_sendcommand(0x0f);
	delay_us(1);
}
void lcd_printf(char c[])
{
	int i=0;
	delay_us(1);//100us
	lcd_sendcommand(0x01);//800us
	delay_us(20);//2ms
	lcd_sendcommand(0x06);//800us
	delay_us(1);//100us
	lcd_sendcommand(0x0f);//800us
	delay_us(1);//100us
	while(c[i]!='\0')
		{
			lcd_senddata(c[i]);//800us perletter
			i++;
		}
}
void lcd_num(uint32_t num)
{
	char c[5];
	sprintf(c,"%d",num);
	lcd_printf(c);
}
