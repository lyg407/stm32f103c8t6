#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

void SysTick_Init(void);
void SysTick_Enable(unsigned char enable);

#endif /* __SYSTICK_H */
