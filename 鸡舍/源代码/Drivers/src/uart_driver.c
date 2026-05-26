/**
 * @file uart_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief
 * @version 0.1
 * @date 2025-12-05
 *
 * @copyright Copyright (c) 2025
 *
 */

// UART1_TX -> PA9
// UART1_RX -> PA10
// usart2 	PA2->TX 	PA3->RX
// usart3	PB10->TX	PB11->RX

#include "uart_driver.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"

// USART1 接收缓冲区
#define USART1_RECV_BUF_SIZE 32
volatile char usart1_recv_buf[USART1_RECV_BUF_SIZE] = "";
volatile int usart1_recv_index = 0; // 终端中像缓冲区存入的索引
volatile int usart1_get_index = 0;	// 应用中从缓冲区往外取的索引
volatile int usart1_idle_flag = 0;

#define USART2_RECV_BUF_SIZE 32
volatile char usart2_recv_buf[USART2_RECV_BUF_SIZE] = "";
volatile int usart2_recv_index = 0; // 终端中像缓冲区存入的索引
volatile int usart2_get_index = 0;	// 应用中从缓冲区往外取的索引
volatile int usart2_idle_flag = 0;

#define USART3_RECV_BUF_SIZE 32
volatile char usart3_recv_buf[USART3_RECV_BUF_SIZE] = "";
volatile int usart3_recv_index = 0; // 终端中像缓冲区存入的索引
volatile int usart3_get_index = 0;	// 应用中从缓冲区往外取的索引
volatile int usart3_idle_flag = 0;

/**
 * @brief 初始化usart1
 *
 * @param baudrate 波特率
 */
void usart1_init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	// 使能GPIO和USART时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	// 配置 PA9 为复用推挽输出
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// 配置 PA10 为上拉输入
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// 初始化 USART1
	USART_InitStruct.USART_BaudRate = baudrate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无控制流
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				 // 既收又发
	USART_InitStruct.USART_Parity = USART_Parity_No;							 // 无奇偶校验位
	USART_InitStruct.USART_StopBits = USART_StopBits_1;							 // 一位停止位
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;					 // 8位数据位
	USART_Init(USART1, &USART_InitStruct);

	// 使能USART1
	USART_Cmd(USART1, ENABLE);

	// 外部中断配置
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;		   // 使能中断
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;		   // 响应优先级
	NVIC_Init(&NVIC_InitStruct);

	// 使能USART1的 RXNE 和 IDLE 中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
}

/**
 * @brief USART1 接收数据
 *
 * @return int 收到的数据
 */
int usart1_getchar(void)
{
	int data;
	// 等待usart1可用
	while (usart1_get_index >= usart1_recv_index)
		;
	// 从缓冲区中读数据
	data = usart1_recv_buf[usart1_get_index++];

	// 读完清零
	if (usart1_get_index >= usart1_recv_index)
		usart1_get_index = usart1_recv_index = usart1_idle_flag = 0;
	usart1_putchar(data); // 回显
	return data;
}

int usart1_putchar(int data)
{
	// 等待可发送数据
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		;

	// 发送数据
	USART_SendData(USART1, data);

	// 返回发送的数据
	return data;
}

// 重映射fgetc等库函数
#include <stdio.h>
#include <time.h>
// #include <rt_misc.h>

#pragma import(__use_no_semihosting_swi)

// ÆÁ±ÎÒÔÏÂÉùÃ÷
// extern int  sendchar(int ch);  /* in Serial.c */
// extern int  getkey(void);      /* in Serial.c */
extern long timeval; /* in Time.c   */

struct __FILE
{
	int handle; /* Add whatever you need here */
};
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
	// return (sendchar(ch));
	return usart1_putchar(ch); // ¸ÄÎª usart1_putchar()
}

int fgetc(FILE *f)
{
	// return (sendchar(getkey()));
	return usart1_getchar(); // ¸ÄÎª usart1_getchar()
}

int ferror(FILE *f)
{
	/* Your implementation of ferror */
	return EOF;
}

void _ttywrch(int ch)
{
	// sendchar (ch);
	usart1_putchar(ch); // ¸ÄÎª usart1_putchar()
}

void _sys_exit(int return_code)
{
	while (1)
		; /* endless loop */
}

void usart_else_init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	// 使能 GPIOA/GPIOB 和 USART2/USART3时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3, ENABLE);

	// 配置 PA2/PB10 为复用推挽输出
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// 配置 PA3/PB11 为上拉输入
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	// 初始化 USART2/USART3
	USART_InitStruct.USART_BaudRate = baudrate;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无控制流
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				 // 既收又发
	USART_InitStruct.USART_Parity = USART_Parity_No;							 // 无奇偶校验位
	USART_InitStruct.USART_StopBits = USART_StopBits_1;							 // 一位停止位
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;					 // 8位数据位
	// void USART_Init(USART_TypeDef* USARTx, USART_InitTypeDef* USART_InitStruct);
	USART_Init(USART2, &USART_InitStruct);
	USART_Init(USART3, &USART_InitStruct);

	// 使能 USART2
	// void USART_Cmd(USART_TypeDef* USARTx, FunctionalStatea NewState);
	USART_Cmd(USART2, ENABLE);
	USART_Cmd(USART3, ENABLE);

	// NVIC 初始化
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;		   // ÖÐ¶ÏÏòÁ¿±íÈë¿Ú
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;		   // Ê¹ÄÜÖÐ¶Ï£¬USART1 ×Ü¿ª¹Ø
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2; // ÇÀÕ¼ÓÅÏÈ¼¶
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;		   // ÏìÓ¦ÓÅÏÈ¼¶
	// void NVIC_Init(NVIC_InitTypeDef* NVIC_InitStruct);
	NVIC_Init(&NVIC_InitStruct);

	// 使能 USART2 RXNE 和 IDLE 中断
	// void USART_ITConfig(USART_TypeDef* USARTx, uint16_t USART_IT, FunctionalState NewState);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

	// 使能 USART3 RXNE 和 IDLE 中断
	NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
	NVIC_Init(&NVIC_InitStruct);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
}

int usart2_getchar(void)
{
	int data;
	while (usart2_get_index >= usart2_recv_index)
		;
	data = usart2_recv_buf[usart2_get_index++];
	if (usart2_get_index >= usart2_recv_index)
	{
		usart2_get_index = usart2_recv_index = usart2_idle_flag = 0;
	}
	return data;
}

int usart3_getchar(void)
{
	int data;
	while (usart3_get_index >= usart3_recv_index)
		;
	data = usart3_recv_buf[usart3_get_index++];
	if (usart3_get_index >= usart3_recv_index)
	{
		usart3_get_index = usart3_recv_index = usart3_idle_flag = 0;
	}
	return data;
}

int usart2_putchar(int data)
{
	uint32_t timeout = 10000; // 超时计数，避免无限等待

	// 等待可发送数据
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
	{
		if (--timeout == 0)
		{
			printf("USART2 TX timeout!\n");
			return -1; // 发送超时
		}
	}

	// 发送数据
	USART_SendData(USART2, data);

	// 返回发送的数据
	return data;
}

int usart3_putchar(int data)
{
	// 等待可发送数据
	while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		;

	// 发送数据
	USART_SendData(USART3, data);

	// 返回发送的数据
	return data;
}
