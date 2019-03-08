/**
  ******************************************************************************
  * @file    bsp_usart.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   调试用的printf串口，重定向printf到串口
  ******************************************************************************
  * @attention
  *
  * 实验平台:秉火 F103-指南者 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 


#include "./usart/bsp_usart.h"

#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 

void _ttywrch(int ch)
{
	ch = ch;
}

///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}

#endif

int console_write(const char *format, ...)
{
	va_list arg;
	int done;

	va_start (arg, format);
	done = vfprintf(stdout, format, arg);
	va_end (arg);

	return done;
}

char console_readChar(void)
{
	char c;
	
	while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);
	c = USART_ReceiveData(DEBUG_USARTx);

	return c;
}

int console_readLine(char* const c, const size_t n)
{
	char ch = ' ';
	size_t i = 0;

	while (ch != '\r')
	{
		ch = console_readChar();
		c[i++] = ch;
	}
	c[i-1] = '\0';
	return i;
}

char console_kbhit(void)
{
	char c;

	c = USART_ReceiveData(DEBUG_USARTx);
	return c;
}

static void console_lowlevel_write(uint8_t const* buf, size_t bufSize)
{
	for (size_t i = 0; i < bufSize; ++i)
	{
		USART_SendData(DEBUG_USARTx, (uint8_t) buf[i]);
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);
		if (buf[i] == '\n')
		{
			USART_SendData(DEBUG_USARTx, (uint8_t) '\r');
			/* 等待发送完毕 */
			while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);	
		}
	}
}

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------------------------
// IAR specific code to retarget output stream to serial port
//----------------------------------------------------------------------------
#if defined(__ICCARM__)
size_t __write(int handle, const unsigned char* buf, size_t bufSize)
{
	console_lowlevel_write((uint8_t const*)buf, bufSize);
	return bufSize;
}
#endif // IAR
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// GCC (newlib nano) specific code to retarget output stream to serial port 
//----------------------------------------------------------------------------
#if defined(__GNUC__)
_ssize_t _write (int file, const void *ptr, size_t bufSize)
{
	unsigned char const* buf = (unsigned char const*)ptr;
	console_lowlevel_write(buf, bufSize);
	return bufSize;
}
#endif // GCC
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
// Keil specific code to retarget output stream to serial port
//----------------------------------------------------------------------------
#if (defined(__arm__) && defined(__ARMCC_VERSION))

#include <rt_sys.h>

const char __stdin_name[] =  ":tt";
const char __stdout_name[] =  ":tt";
const char __stderr_name[] =  ":tt";

FILEHANDLE _sys_open(const char *name, int openmode) { return 1; }
int _sys_close(FILEHANDLE fh) { return 0; }
int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode) { return -1; }
int _sys_istty(FILEHANDLE fh) { return 0; }
int _sys_seek(FILEHANDLE fh, long pos) { return -1; }
long _sys_flen(FILEHANDLE fh) { return -1; }

int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode)
{
	console_lowlevel_write((uint8_t const*)buf, (size_t)len);
	return 0;
}

void _ttywrch(int ch)
{
	uint8_t c = (uint8_t)ch;
	console_lowlevel_write(&c, 1);
}
#endif //Keil
//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif


///重定向c库函数printf到串口，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到串口，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}


 /**
  * @brief  USART GPIO 配置,工作参数配置
  * @param  无
  * @retval 无
  */
void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开串口GPIO的时钟
	DEBUG_USART_GPIO_APBxClkCmd(DEBUG_USART_GPIO_CLK, ENABLE); 
	
	// 打开串口外设的时钟
	DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE);

	// 将USART Tx的GPIO配置为推挽复用模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // 将USART Rx的GPIO配置为浮空输入模式
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// 配置串口的工作参数
	// 配置波特率
	USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
	// 配置 针数据字长
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// 配置停止位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// 配置校验位
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// 配置硬件流控制
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// 配置工作模式，收发一起
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// 完成串口的初始化配置
	USART_Init(DEBUG_USARTx, &USART_InitStructure);

	// 使能串口
	USART_Cmd(DEBUG_USARTx, ENABLE);	    
}



