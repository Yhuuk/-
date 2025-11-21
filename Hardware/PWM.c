#include "stm32f10x.h"                  // Device header

/**
  * 函    数：PWM初始化
  * 参    数：无
  * 返 回 值：无
  */
void PWM_Init(void)
{
	/*开启时钟*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM2);		//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 112;					//计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 9;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);          //清除更新中断标志位
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);    //使能更新中断

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);      //设置中断优先级分组2
	NVIC_InitTypeDef NVIC_InitStructure;                 //定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;      //选择TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;   //响应优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;      //使能中断
	NVIC_Init(&NVIC_InitStructure);                      //初始化NVIC寄存器
	
	/*TIM使能*/
	TIM_Cmd(TIM2, ENABLE);			//使能TIM2，定时器开始运行
}

