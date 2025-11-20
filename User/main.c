#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "IC.h"
#include "LED.h"
#include "FSM_Control.h"


/*****************************************************************************
 * 4个LED同时常亮的电流是280mA
 * 
 * 
 * 
******************************************************************************/
int main(void)
{

	LED_Init();
	IC_Init();			//输入捕获初始化
	TIM3_Init();
	
	while (1)
	{
		LED_Switch();
		
	}
}
