#include "FSM_Control.h"

bool state_flag = 0;
bool first_gear_flag = 0;
FSM_State current_state = STATE_NULL;
FSM_State first_gear_state = STATE_NULL;

void FSM_Init(void)
{
    // FSM Initialization code here
    switch (current_state)
    {
        case STATE_NULL:
        {
            /* code */
            if(TIM_GetCapture2(TIM1) >=1850 && TIM_GetCapture2(TIM1) <2200) //二档
            {
                current_state = STATE_DOWN;
            }
            break;
        }
        case STATE_DOWN:
        {
            /* code */
            if(TIM_GetCapture2(TIM1) >=1850 && TIM_GetCapture2(TIM1) <2200)
            {
                state_flag = 1;
                current_state = STATE_RELEASE;
            }  
            break;
        }
        case STATE_RELEASE:
        {
            /* code */
            if(TIM_GetCapture2(TIM1) >= 1450 && TIM_GetCapture2(TIM1) <1800) //二档
            {
                current_state = STATE_NULL;
            }
            break;
        }
        default:
            break;
    }
}

void FSM_Init_First_Gear(void)
{
    // FSM Initialization code here
    switch (first_gear_state)
    {
        case STATE_NULL:
        {
            /* code */
            if(TIM_GetCapture2(TIM1) >=1450 && TIM_GetCapture2(TIM1) <1800) //二档
            {
                first_gear_state = STATE_DOWN;
            }
            break;
        }
        case STATE_DOWN:
        {
            /* code */
            if(TIM_GetCapture2(TIM1) >=1450 && TIM_GetCapture2(TIM1) <1800)
            {
                first_gear_flag = 1;
                first_gear_state = STATE_RELEASE;
            }  
            break;
        }
        case STATE_RELEASE:
        {
            /* code */
            if(TIM_GetCapture2(TIM1) >= 950 && TIM_GetCapture2(TIM1) <1400) //二档
            {
                first_gear_state = STATE_NULL;
            }
            break;
        }
        default:
            break;
    }
}


void TIM3_Init(void)
{
    // TIM3 Initialization code here
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;             //1ms一次中断
    TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_ClearFlag(TIM3, TIM_FLAG_Update);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        // Handle the timer interrupt
        FSM_Init();
        FSM_Init_First_Gear();
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}