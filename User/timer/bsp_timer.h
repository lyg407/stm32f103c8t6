#ifndef TIME_TEST_H
#define TIME_TEST_H

#include "stm32f10x.h"
#if defined(BSP_TIMER_USE_SYSTICK)
#include "bsp_SysTick.h"
#endif
 
//#define START_TIME  time=0;RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);TIM_Cmd(TIM2, ENABLE)
//#define STOP_TIME  TIM_Cmd(TIM2, DISABLE);RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE)
enum
{
	BSP_TIMER_0,
	BSP_TIMER_1,
	BSP_TIMER_2,

	BSP_TIMER_MAX
};

typedef struct
{
	//unsigned short millTime;
	unsigned char  used;
	unsigned short irqCount;
	unsigned short irqCountMax;
	void (*callback)(int timeId);
} bsp_timer_t;

void TIM2_NVIC_Configuration(void);
void TIM2_Configuration(void);
void TIM3_NVIC_Configuration(void);
void TIM3_Configuration(void);
void TIM4_NVIC_Configuration(void);
void TIM4_Configuration(void);

void bsp_timer_hw_init(void);
void bsp_timer_hw_enable(int timerId, unsigned char flag);
void bsp_start_timer(int timerId, unsigned short millTime, void (*callback)(int id));
void bsp_stop_timer(int timerId);
void bsp_timer_proc(void);
#if defined(BSP_TIMER_USE_SYSTICK)
void bsp_timer_systick_enable(int timerId, unsigned char flag);
#endif

#endif	/* TIME_TEST_H */
