#ifndef __STM32L476G_DISCOVERY_CLOCK_H
#define __STM32L476G_DISCOVERY_CLOCK_H

#include "stm32l476xx.h"

void System_Clock_Init(void);
void init_tim5_clock(void);
void Run_Capture(void);
int Check_Capture(void);

#endif /* __STM32L476G_DISCOVERY_DMA_H */



