#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "FSM_Control.h"
#include <stdbool.h>

void LED_Init(void);


typedef enum
{ OFF =  Bit_RESET,
  ON = Bit_SET
} LED_State;

void LED1_State(LED_State state);

void LED_initial_mode(void);
void LED_FirstMode(void);
void LED_SecondMode(void);
void LED_ThirdMode(void);
void LED_Switch(void);
void LED_FourthMode(void);

#endif
