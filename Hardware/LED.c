#include "stm32f10x.h"                  // Device header
#include "LED.h"

uint8_t change_mode = 0;
bool stop_flag = 0;

/*---------------- 参数&类型定义 ----------------*/
#define FIRST_GEAR_MIN   950
#define FIRST_GEAR_MAX   1300

typedef enum
{
    LED_MODE_FIRST = 0,
    LED_MODE_SECOND,
    LED_MODE_THIRD,
    LED_MODE_COUNT       // 模式数量，用来做取模
} LED_Mode;

/**
  * 函    数：LED初始化
  * 参    数：无
  * 返 回 值：无
  */
void LED_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);						//将PA1和PA2引脚初始化为推挽输出
	
	/*设置GPIO初始化后的默认电平*/
	GPIO_ResetBits(GPIOA, GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 );				//设置PA1和PA2引脚为高电平
}

void LED1_State(LED_State state)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_1, (BitAction)(state));
}

void LED2_State(LED_State state)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_2, (BitAction)(state));
}

void LED3_State(LED_State state)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_3, (BitAction)(state));
}

void LED4_State(LED_State state)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, (BitAction)(state));
}

/*****************************************
 * LED初始模式：全部熄灭
 * 
 * 
 * *****************************************/
void LED_initial_mode(void)
{
	LED1_State(OFF);
	LED2_State(OFF);
	LED3_State(OFF);
	LED4_State(OFF);
}



/*****************************************************
 *模式1：LED1~LED4全部常亮
 * 
 * 
 * *************************************************/
void LED_FirstMode(void)
{
	LED1_State(ON);
	LED2_State(ON);
	LED3_State(ON);
	LED4_State(ON);
}



/**********************************************************
 * 模式2：LED1和LED4一组，LED2和LED3一组交替闪烁(延时为100ms)
 * 
 * 
 * 
 * ******************************************************/
void LED_SecondMode(void)
{
	LED1_State(ON);
	LED4_State(ON);
	Delay_ms(100);
	LED1_State(OFF);
	LED4_State(OFF);

	LED2_State(ON);
	LED3_State(ON);
	Delay_ms(100);
	LED2_State(OFF);
	LED3_State(OFF);
}

/**********************************************************
 * 模式3：LED1~LED4依次点亮(延时为50ms)
 * 
 * 
 * 
 * ******************************************************/
void LED_ThirdMode(void)
{
	LED1_State(ON);
	Delay_ms(50);
	LED1_State(OFF);

	LED2_State(ON);
	Delay_ms(50);
	LED2_State(OFF);

	LED3_State(ON);
	Delay_ms(50);
	LED3_State(OFF);

	LED4_State(ON);
	Delay_ms(50);
	LED4_State(OFF);
}


/*---------------- 内部小工具函数 ----------------*/

static inline bool is_first_gear(uint16_t pwm)
{
    return (pwm >= FIRST_GEAR_MIN) && (pwm < FIRST_GEAR_MAX);
}


static void LED_ApplyMode(uint8_t mode)
{
    switch (mode)
    {
        case LED_MODE_FIRST:
            LED_FirstMode();
            break;
        case LED_MODE_SECOND:
            LED_SecondMode();
            break;
        case LED_MODE_THIRD:
            LED_ThirdMode();
            break;
        default:
            // 理论上不会走到这里，当保护用
            LED_initial_mode();
            break;
    }
}

/*---------------- 主逻辑函数 ----------------*/

void LED_Switch(void)
{
    uint16_t ch_val = TIM_GetCapture2(TIM1);

    /********** 1. 根据拨杆位置设置 stop_flag **********/
    if (is_first_gear(ch_val))
    {
        stop_flag = 1;
    }
    else
    {
        stop_flag = 0;
        Delay_ms(5);
    }

    /********** 2. 一档切换成二档触发：恢复到初始模式 **********/
    if (first_gear_flag)
    {
        first_gear_flag = 0;
        change_mode = 0;
    }

    /********** 3. 切换模式：state_flag 上升沿触发 **********/
    if (state_flag)
    {
        state_flag = 0;

        change_mode++;
        if (change_mode >= LED_MODE_COUNT)
        {
            change_mode = 0;
        }
    }

    /********** 4. 根据 stop_flag/模式，实际执行灯效 **********/
    if (stop_flag)
    {
        // 拨杆在“一档” -> 进入初始模式（全部熄灭 或 某种安全模式）
        LED_initial_mode();
    }
    else
    {
        // 按当前模式执行灯效
        LED_ApplyMode(change_mode);
    }
}
