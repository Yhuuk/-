#include "stm32f10x.h"                  // Device header
#include "LED.h"

uint8_t change_mode = 0;
bool stop_flag = 0;

/*---------------- 参数&类型定义 ----------------*/
#define FIRST_GEAR_MIN   950
#define FIRST_GEAR_MAX   1300

// 呼吸亮度等级：0~64，总共 65 档，占空比更细腻
#define BREATH_LEVELS      64      

// 软件 PWM 周期：1kHz → 1ms = 1000us
#define PWM_PERIOD_US      1000    

// 每一级亮度持续的时间（下面会详细说怎么选）
#define STEP_TIME_MS       30      

// 最亮保持时间 & 熄灭保持时间
#define BREATH_ON_HOLD_MS  1000    
#define BREATH_OFF_HOLD_MS 1000  

/*---------------- 软件PWM+定时器参数 ----------------*/
static uint64_t duration_time = 0;   //持续时间20ms，从10us的累计到2000us duration_time最大为2000

uint8_t on_flag = 0;
uint8_t current_duty = 1;     //当前占空比
uint8_t cycle_count = 99;    //周期总数
#define DutyCycle_Max 100  // 最大占空比，对应100%



typedef enum
{
    LED_MODE_FIRST = 0,
    LED_MODE_SECOND,
    LED_MODE_THIRD,
    LED_MODE_FOURTH,
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


// 软件 PWM：在 duration_ms 时间内，按亮度 level 做“亮灭循环”
// level: 0 ~ breathing_count
static void LED1_SoftwarePWM_Level(uint8_t level, uint16_t duration_ms)
{
    uint32_t elapsed_us  = 0;
    uint32_t duration_us = (uint32_t)duration_ms * 1000;
    uint16_t on_us, off_us;

    if (BREATH_LEVELS == 0) return;
    if (level > BREATH_LEVELS) level = BREATH_LEVELS;

    while (elapsed_us < duration_us)
    {
        // 占空比 = level / BREATH_LEVELS
        on_us  = (uint16_t)((uint32_t)PWM_PERIOD_US * level / BREATH_LEVELS);
        off_us = PWM_PERIOD_US - on_us;

        // 避免 on_us/off_us 都为 0 导致死循环
        if (on_us == 0 && off_us == 0) {
            off_us = 10;
        }

        if (on_us > 0) {
            LED1_State(ON);
            Delay_us(on_us);
            elapsed_us += on_us;
            if (elapsed_us >= duration_us) break;
        }

        if (off_us > 0) {
            LED1_State(OFF);
            Delay_us(off_us);
            elapsed_us += off_us;
        }
    }
}

void LED_FourthMode(void)
{

   uint8_t  level;

    // 1. 渐亮：亮度从 0 → BREATH_LEVELS
    for (level = 0; level <= BREATH_LEVELS; level++)
    {
        LED1_SoftwarePWM_Level(level, STEP_TIME_MS);
    }

    // 2. 最亮保持
    LED1_State(ON);
    Delay_ms(BREATH_ON_HOLD_MS);

    // 3. 渐暗：亮度从 BREATH_LEVELS → 0
    for (level = BREATH_LEVELS; level > 0; level--)
    {
        LED1_SoftwarePWM_Level(level, STEP_TIME_MS);
    }

    // 4. 熄灭保持
    LED1_State(OFF);
    Delay_ms(BREATH_OFF_HOLD_MS);
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
        case LED_MODE_FOURTH:
            LED_FourthMode();
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

// 10us进一次
void TIM2_IRQHandler(void)  
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的中断发生与否：TIM2更新中断
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清除TIM2更新中断标志位

		// 在这里添加你的中断处理代码
        
        on_flag ++;

        if(on_flag <= current_duty)
        {
            // 点亮LED
            LED1_State(ON);
        }
        else
        {
            // 熄灭LED
            LED1_State(OFF);
        }
        
        duration_time++;

        if(duration_time > 2000)  //20ms周期
        {
            duration_time = 0;
            current_duty ++;     //每20ms占空比加1
            on_flag = 0;
        }
		
	}
}
