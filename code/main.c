#include"LCD.h"
#include<stdint.h>

uint32_t input;
uint32_t current_state;
uint32_t prev;
uint32_t count=0;

struct state{
    uint32_t out;
    uint32_t wait;
    uint32_t next[8];
};
#define allStopWest 0
#define greenSouth 1
#define yellowSouth 2
#define allStopSouth 3
#define walk 4
#define redOn 5
#define redOff 6
#define redOn2 7
#define redOff2 8
#define allStopWalk 9
#define greenWest 10
#define yellowWest 11
#define standByS 12
#define standByW 13
#define standByWalk 14
#define white 15
struct state FSM[16] ={
    {0x4C, 0, {allStopWest, greenWest, walk, walk, greenSouth, greenSouth, greenSouth, greenSouth}},//allStopWest 0
    {0x64, 10, {yellowSouth, yellowSouth, yellowSouth, yellowSouth, standByS, yellowSouth, yellowSouth, yellowSouth}},//greenSouth 1
    {0x54, 5, {allStopSouth,allStopSouth,allStopSouth,allStopSouth,allStopSouth,allStopSouth,allStopSouth,allStopSouth}},//yellowSouth 2
    {0x4C, 0, {allStopSouth, greenWest, walk, walk, greenSouth, greenWest, walk, walk}},//allStopSouth 3
    {0x8C, 10, {white, white, standByWalk, white, white, white, white, white}},//walk 4
    {0x4C, 2, {redOff, redOff,redOff,redOff,redOff,redOff,redOff,redOff}},//redOn 5
    {0xC, 3, {redOn2, redOn2, redOn2,redOn2,redOn2,redOn2,redOn2,redOn2}},//redOff 6
    {0x4C, 4, {redOff2, redOff2,redOff2,redOff2,redOff2,redOff2,redOff2,redOff2}},//redOn2 7
    {0xC, 5, {allStopWalk,allStopWalk,allStopWalk,allStopWalk,allStopWalk,allStopWalk,allStopWalk,allStopWalk}},//redOff2 8
    {0x4C, 0, {allStopWalk, greenWest, walk, greenWest, greenSouth, greenWest, greenSouth, greenWest}},//allStopWalk 9
    {0x49, 10, {yellowWest, standByW, yellowWest,yellowWest,yellowWest,yellowWest,yellowWest,yellowWest,}},//greenWest 10
    {0x4A, 5, {allStopWest,allStopWest,allStopWest,allStopWest,allStopWest,allStopWest,allStopWest,allStopWest}},//yellowWest 11
		{0x64, 0, {yellowSouth, yellowSouth, yellowSouth, yellowSouth, standByS, yellowSouth, yellowSouth, yellowSouth}},//standByS 12
    {0x49, 0, {yellowWest, standByW, yellowWest,yellowWest,yellowWest,yellowWest,yellowWest,yellowWest,}},//standByW 13
		{0x8C, 0, {white, white, standByWalk, white, white, white, white, white}},//standByWalk 14
		{0x8C, 1, {redOn, redOn, redOn, redOn, redOn, redOn, redOn, redOn}},//white 15
};
void EXTI9_5_IRQHandler()
{
	if(EXTI->PR&0xE0)
	{
		input = (GPIOB->IDR >> 5) & 0x07;
	}
	EXTI->PR|=0xE0;
}
void Delay1s(unsigned int time) // Delay time*1s
{	
	for(uint32_t j=0;j<10000;j++) {
		unsigned int i=0;
		while(i<time)
		{
			SysTick->VAL=0;
			while(! (SysTick->CTRL&(1<<16)))
			{}
			i++;
		}
	}
}
void waiting(unsigned int time) // Delay time*100us and LCD print out
{
	uint32_t t=10000*time;
	while(count<t)
	{
		SysTick->VAL=0;
		while(! (SysTick->CTRL&(1<<16)))
		{}
		if (count%10000==0) {
			lcd_num(1+count/10000);
		}
		count++;
	}
}

int main(void){
  SysTick_Init();
  // GPIO things ...
	RCC->APB2ENR=0x0D;//enable port A B and AFIO
	GPIOA->CRL=0x11111111; //A0-7
	GPIOA->ODR=0xFF;
	GPIOB->CRL=0x88800000; //B5,6,7 switch
	GPIOB->ODR=0x00; // pull down
	
	AFIO->EXTICR2=0x1110;

	EXTI->IMR |= 0xE0; // Interupt pin 5-6-7
	EXTI->RTSR|= 0xE0; // Interupt pin 5-6-7
	EXTI->FTSR|= 0xE0; // Interupt pin 5-6-7
	NVIC->ISER[0]|=1<<23; // EXTI9_5_IRQHandler
	// LCD GPIO
	GPIOB->CRH=0x11110000; // B12-15 tuong ung voi D4-7 cua LCD
	GPIOB->ODR=0x00;
	GPIOA->CRH=0x011;			// A8-RS // A9-E
	lcd_initialize();
  current_state = 0;
	input=0;
	lcd_printf("00");
  while(1){
        // 1) Output
        GPIOA->ODR = (GPIOA->ODR & (~0xFF)) | FSM[current_state].out; // Output led
        
        // 2) Wait
        waiting(FSM[current_state].wait);
				
        // 3) Read input polling
				//input = (GPIOB->IDR >> 5) & 0x07;
        // 4) Next state
				prev=current_state;
        current_state = FSM[current_state].next[input];
		// 00 if all stop
				if((current_state==0 || current_state==3 || current_state==9) && current_state==prev) {
					// do nothing
				}else {
				lcd_printf("00");
				}
		 // continue to counting
				if(current_state>=12 && current_state<=14) {
					count+=10000;
					lcd_num(count/10000);
					Delay1s(1);
				}
				else if ((current_state>=5 && current_state<=8)) {}
				else {
					count = 0;
				}
	}
}
