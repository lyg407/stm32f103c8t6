/**************************************
 * 文件名  ：Time_test.c
 * 描述    ：TIM2 1ms 定时应用函数库
 *          
 * 实验平台：MINI STM32开发板 基于STM32F103C8T6
 * 硬件连接：无
 * 库版本  ：ST3.5.0  

*********************************************************/	
#include "bsp_timer.h"

#define BSP_IRQ_TIME	1			// ms

static bsp_timer_t	bsp_timer_array[BSP_TIMER_MAX];

/* TIM2中断优先级配置 */
void bsp_timer_array_init(void)
{
	int i;

	for(i=0; i<BSP_TIMER_MAX; i++)
	{
		bsp_timer_array[i].used = 0;
		bsp_timer_array[i].irqCount = 0;
		bsp_timer_array[i].irqCountMax = 0;
		bsp_timer_array[i].callback = 0;
	}
}

void bsp_timer_update(int timerId)
{
	bsp_timer_array[timerId].irqCount++;
}

void bsp_timer_update_all(void)
{	
	int timerId=0;
	
	while(timerId < BSP_TIMER_MAX)
	{
		if(bsp_timer_array[timerId].used)
			bsp_timer_array[timerId].irqCount++;
		timerId++;
	}
}

void bsp_start_timer(int timerId, unsigned short millTime, void (*callback)(int id))
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	bsp_timer_array[timerId].irqCount = 0;
	if(millTime%BSP_IRQ_TIME)
		bsp_timer_array[timerId].irqCountMax = ((unsigned short)(millTime/BSP_IRQ_TIME))+1;
	else
		bsp_timer_array[timerId].irqCountMax = ((unsigned short)(millTime/BSP_IRQ_TIME));
	
	bsp_timer_array[timerId].callback = callback;
#if defined(BSP_TIMER_USE_SYSTICK)
	bsp_timer_systick_enable(timerId, 1);
#else
	bsp_timer_hw_enable(timerId, 1);
#endif
	bsp_timer_array[timerId].used = 1;
}

void bsp_stop_timer(int timerId)
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	bsp_timer_array[timerId].used = 0;
	bsp_timer_array[timerId].callback = 0;
#if defined(BSP_TIMER_USE_SYSTICK)
	bsp_timer_systick_enable(timerId, 0);
#else
	bsp_timer_hw_enable(timerId, 0);
#endif
}

void bsp_timer_proc(void)
{
	int i;

	for(i=0; i<BSP_TIMER_MAX; i++)
	{
		if(bsp_timer_array[i].used)
		{
			if(bsp_timer_array[i].irqCount >= bsp_timer_array[i].irqCountMax)
			{
				bsp_timer_array[i].irqCount = 0;
				if(bsp_timer_array[i].callback)
				{
					//console_write("timer 0 callback \n");
					bsp_timer_array[i].callback(i);
				}
			}
		}
	}
}


void TIM2_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*中断周期为1ms*/
void TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Period=1000*BSP_IRQ_TIME;	//自动重装载寄存器周期的值(计数值) 
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
    TIM_TimeBaseStructure.TIM_Prescaler= (72 - 1);	//时钟预分频数 72M/72      
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式 
    //TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);	// 清除溢出中断标志 
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2, DISABLE);	// 开启时钟    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);	//先关闭等待使用  
}

void TIM3_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*中断周期为1ms*/
void TIM3_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_DeInit(TIM3);
    TIM_TimeBaseStructure.TIM_Period=1000*BSP_IRQ_TIME;	//自动重装载寄存器周期的值(计数值) 
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
    TIM_TimeBaseStructure.TIM_Prescaler= (72 - 1);	//时钟预分频数 72M/72      
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式 
    //TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);	// 清除溢出中断标志 
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM3, DISABLE);	// 开启时钟    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);	//先关闭等待使用  
}

void TIM4_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*中断周期为1ms*/
void TIM4_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    TIM_DeInit(TIM4);
    TIM_TimeBaseStructure.TIM_Period=1000*BSP_IRQ_TIME;	//自动重装载寄存器周期的值(计数值) 
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
    TIM_TimeBaseStructure.TIM_Prescaler= (72 - 1);	//时钟预分频数 72M/72      
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式 
    //TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
    TIM_ClearFlag(TIM4, TIM_FLAG_Update);	// 清除溢出中断标志 
    TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM4, DISABLE);	// 开启时钟    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, DISABLE);	//先关闭等待使用  
}

void bsp_timer_hw_enable(int timerId, unsigned char flag)
{
	if(timerId >= BSP_TIMER_MAX)
		return;

	if(flag)
	{
		if(timerId == BSP_TIMER_0)
		{
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
			TIM_Cmd(TIM2, ENABLE);
		}	
		else if(timerId == BSP_TIMER_1)
		{
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
			TIM_Cmd(TIM3, ENABLE);
		}
		else if(timerId == BSP_TIMER_2)
		{
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , ENABLE);
			TIM_Cmd(TIM4, ENABLE);
		}
	}
	else
	{	
		if(timerId == BSP_TIMER_0)
		{
			TIM_Cmd(TIM2, DISABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);
		}
		else if(timerId == BSP_TIMER_1)
		{
			TIM_Cmd(TIM3, DISABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , DISABLE);
		}
		else if(timerId == BSP_TIMER_2)
		{
			TIM_Cmd(TIM4, DISABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4 , DISABLE);
		}
	}
}

#if defined(BSP_TIMER_USE_SYSTICK)
void bsp_timer_systick_enable(int timerId, unsigned char flag)
{
	int index;

	if(timerId >= BSP_TIMER_MAX)
		return;

	for(index=0; index<BSP_TIMER_MAX; index++)
	{
		if(bsp_timer_array[index].used)
			break;
	}
	if(index == BSP_TIMER_MAX)
	{
		SysTick_Enable(flag);
	}
}
#endif

void bsp_timer_hw_init(void)
{
#if defined(BSP_TIMER_USE_SYSTICK)
	SysTick_Init();
#else
	TIM2_Configuration();
	TIM2_NVIC_Configuration();

	TIM3_Configuration();
	TIM3_NVIC_Configuration();

	TIM4_Configuration();
	TIM4_NVIC_Configuration();
#endif
	bsp_timer_array_init();
}


