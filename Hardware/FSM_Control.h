#ifndef __FSM_Control_H
#define __FSM_Control_H

#include "stm32f10x.h"
#include <stdbool.h>

extern bool  state_flag;
extern bool first_gear_flag;

typedef enum {
    STATE_NULL,
    STATE_DOWN,
    STATE_RELEASE
} FSM_State;

void TIM3_Init(void);

#endif
