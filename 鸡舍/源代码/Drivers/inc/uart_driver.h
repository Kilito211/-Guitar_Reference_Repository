#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include <stdio.h>

#define USART1_RECV_BUF_SIZE 32
extern volatile char usart1_recv_buf[USART1_RECV_BUF_SIZE];
extern volatile int usart1_recv_index;
extern volatile int usart1_get_index;
extern volatile int usart1_idle_flag;

#define USART2_RECV_BUF_SIZE 32
extern volatile char usart2_recv_buf[USART2_RECV_BUF_SIZE];
extern volatile int usart2_recv_index;
extern volatile int usart2_get_index;
extern volatile int usart2_idle_flag;

#define USART3_RECV_BUF_SIZE 32
extern volatile char usart3_recv_buf[USART3_RECV_BUF_SIZE];
extern volatile int usart3_recv_index;
extern volatile int usart3_get_index;
extern volatile int usart3_idle_flag;

// USART1
void usart1_init(uint32_t baudrate);
int usart1_putchar(int data);
int usart1_getchar(void);

// USART2
void usart_else_init(uint32_t baudrate);
int usart2_putchar(int data);
int usart2_getchar(void);

// USART3
int usart3_putchar(int data);
int usart3_getchar(void);

#endif
