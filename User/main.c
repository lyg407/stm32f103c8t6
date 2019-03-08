/**
  ******************************************************************************
  * @file    main.c
  * @author  LYC
  * @version V1.0
  * @date    2014-04-22
  * @brief   MPU6050 硬件IIC测试
  ******************************************************************************
  * @attention
  * 实验平台:秉火 指南者 STM32 开发板 
  ******************************************************************************
  */
  
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "stm32f10x_spi.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_usart.h"
#include "qmaX981.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "qst_sw_i2c.h"
#include "bsp_rtc.h"
#include "bsp_timer.h"
#include <math.h>
#if defined(OLED_SUPPORT)
#include "oled.h"
#include "bmp.h"
#endif

//#define MCU_SLEEP_SUPPORT

#define TEST_ACC
//#define TEST_MAG
//#define TEST_PRESS
//#define TEST_FIS2108
//#define TEST_BMI160
//#define TEST_JHM1200


static __IO uint32_t qst_sensor_count = 0;

#if defined(MCU_SLEEP_SUPPORT)
typedef enum
{
	MCU_MODE_NORMAL,
	MCU_MODE_STANDBY,
	MCU_MODE_SLEEP,
	MCU_MODE_STOP,

	MCU_MODE_TOTAL
}mcu_mode_e;

static mcu_mode_e g_mcu_mode = MCU_MODE_NORMAL;
#endif

struct rtc_time systmtime=
{
	0,0,0,1,1,2019,2
};

typedef enum
{
	
	QST_DEVICE_UNKNOW = 0x00,
	QST_DEVICE_ACC = 0x01,
	QST_DEVICE_MAG = 0x02,
	QST_DEVICE_PRESS = 0x04,
	QST_DEVICE_GYRO = 0x08,

	QST_DEVICE_TOTAL = 0xff
}qst_device_e;

static qst_device_e qst_device_type = QST_DEVICE_UNKNOW;

extern __IO uint32_t TimeDisplay ;
extern void qst_func_cyc_timer(void);
extern s32 qmcX983_init(void);
extern int qmcX983_read_mag_xyz(float *data);
extern void qmcX983_get_orientation(float *yaw);
extern u8 qmp6988_init(void);
extern void qmp6988_calc_pressure(void);

extern uint8_t fis210x_init(void);
extern void fis210x_read_acc_xyz(void);
extern void fis210x_read_gyro_xyz(void);

extern unsigned char bmi160_init(void);
//extern void bmi160_set_cali(void);
extern void bmi160_cali_process(void);
extern void bmi160_read_acc_xyz(void);
extern void bmi160_read_gyro_xyz(void);

extern void Virtual_I2C_Init(void);
extern unsigned char jhm1200_init(void);
extern void jhm1200_get_raw_data(void);
extern void JHM1200_get_cal(void);
#if defined(MCU_SLEEP_SUPPORT)
void mcu_entry_sleep(mcu_mode_e mode);
void mcu_exit_sleep(void);
#endif

void qst_delay(unsigned int delay)
{
	int i,j;
	for(i=0;i<delay;i++)
	{
		for(j=0;j<1000;j++)
		{
			;
		}
	}
}

void mcu_timer_sensor_cbk(int timeId)
{
	int acc_data[3];
	
	if(qst_device_type & QST_DEVICE_ACC)
	{
		qmaX981_read_acc(acc_data);
		//console_write("%d acc %f %f %f\n",qst_sensor_count, acc_data[0]/1000.0f, acc_data[1]/1000.0f, acc_data[2]/1000.0f);
#if defined(OLED_SUPPORT)
		{
			u8 buf[100];
			sprintf((char*)buf,"%f %f %f", acc_data[0]/1000.0f, acc_data[1]/1000.0f, acc_data[2]/1000.0f);
			OLED_Clear();
			OLED_ShowString(0,0,buf, 5);
		}
#endif
	}
	if(qst_device_type & QST_DEVICE_PRESS)
	{
		qmp6988_calc_pressure();
	}
	else
	{
		console_write("no sensor !!!\n");
#if defined(OLED_SUPPORT)
		OLED_Clear();
		OLED_ShowString(0,0,"no sensor !", 8);
#endif
	}
}

void mcu_reset_counter(void)
{
	qst_sensor_count = 0;
}

#if defined(MCU_SLEEP_SUPPORT)
void mcu_entry_sleep(mcu_mode_e mode)
{
	console_write("mcu_entry_sleep mode=%d !!!\n", mode);
	bsp_led_set(0);
#if defined(OLED_SUPPORT)
	OLED_Clear();
#endif
	bsp_stop_timer(0);
	bsp_stop_timer(1);
	bsp_stop_timer(2);

	g_mcu_mode = mode;
	if(mode == MCU_MODE_STANDBY)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
		PWR_ClearFlag(PWR_FLAG_WU);	/*清除WU状态位*/
		PWR_WakeUpPinCmd(ENABLE);		/* 使能WKUP引脚的唤醒功能 ，使能PA0*/
		PWR_EnterSTANDBYMode();			/* 进入待机模式 */
	}
	else if(mode == MCU_MODE_SLEEP)
	{
		__WFI();
	}
	else if(mode == MCU_MODE_STOP)
	{
		PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
	}
}

void mcu_exit_sleep(void)
{
	if(g_mcu_mode == MCU_MODE_STOP)
	{
		RCC_HSEConfig(RCC_HSE_ON);	/* 使能 HSE */
		/* 等待 HSE 准备就绪 */
		while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
		{
		}
		RCC_PLLCmd(ENABLE);/* 使能 PLL */ 
		/* 等待 PLL 准备就绪 */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);	/* 选择PLL作为系统时钟源 */
		/* 等待PLL被选择为系统时钟源 */
		while (RCC_GetSYSCLKSource() != 0x08)
		{
		}
		//因为刚唤醒的时候使用的是HSI时钟，会影响串口波特率，输出不对，所以在重新配置时钟源后才使用串口输出。
		console_write("mcu_exit_sleep from stop mode \r\n");
	}
	qst_sensor_count = 0;
	g_mcu_mode = MCU_MODE_NORMAL;

	bsp_start_timer(2, 500, mcu_timer_sensor_cbk);
}
#endif

/**
  * @brief  主函数
  * @param  无  
  * @retval 无
  */
int main(void)
{
	s32 ret=0;
	
	//SysTick_Init();
	LED_GPIO_Config();
	USART_Config();
#if defined(USE_SW_IIC)
	i2c_sw_gpio_config();
#else
	I2C_Bus_Init();
#endif
#if defined(USE_SPI)
	Spi_Init();
#endif
	qst_delay(1000);

//	RTC_NVIC_Config();
//	RTC_CheckAndConfig(&systmtime);
	bsp_timer_hw_init();
#if defined(OLED_SUPPORT)
	OLED_Init();
	OLED_ShowString(0,1,"QST Corp Demo",16);
#endif

	console_write("qst sensor entry %s-%s\n", __DATE__, __TIME__);
	if(qmaX981_init())
	{
		qst_device_type |= QST_DEVICE_ACC;
	}
	if(qmp6988_init())
	{
		qst_device_type |= QST_DEVICE_PRESS;
	}

	if(qst_device_type)
	{
#if defined(MCU_SLEEP_SUPPORT)
		mcu_exit_sleep();
#else		
		bsp_start_timer(2, 500, mcu_timer_sensor_cbk);
#endif
		while(1)
		{
			bsp_timer_proc();
			
			qst_sensor_count++;
			if(qst_sensor_count >= 3000000)		//72000000*3
			{
				qst_sensor_count = 0;
#if defined(MCU_SLEEP_SUPPORT)
				mcu_entry_sleep(MCU_MODE_STOP);
				mcu_exit_sleep();
#endif
			}
		}
	}
	else
	{
		bsp_start_timer(2, 500, mcu_timer_sensor_cbk);
		while(1)
		{
			bsp_timer_proc();
		}
	}
}
/*********************************************END OF FILE**********************/
