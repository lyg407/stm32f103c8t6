/**
  ******************************************************************************
  * @file    bsp_usart.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   �����õ�printf���ڣ��ض���printf������
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:���� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 


#include "./usart/bsp_usart.h"

#if 0
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef�� d in stdio.h. */ 
FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 

void _ttywrch(int ch)
{
	ch = ch;
}

///�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		/* �ȴ������������� */
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
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);
		if (buf[i] == '\n')
		{
			USART_SendData(DEBUG_USARTx, (uint8_t) '\r');
			/* �ȴ�������� */
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


///�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}


 /**
  * @brief  USART GPIO ����,������������
  * @param  ��
  * @retval ��
  */
void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	DEBUG_USART_GPIO_APBxClkCmd(DEBUG_USART_GPIO_CLK, ENABLE); 
	
	// �򿪴��������ʱ��
	DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(DEBUG_USARTx, &USART_InitStructure);

	// ʹ�ܴ���
	USART_Cmd(DEBUG_USARTx, ENABLE);	    
}



