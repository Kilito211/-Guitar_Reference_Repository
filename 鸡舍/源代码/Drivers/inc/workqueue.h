#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#include <stdio.h>
#include <stdbool.h>
#include "stm32f10x.h"
#include "typedef.h"
#include "uart_driver.h"
#include "systick_driver.h"
#include "timer_driver.h"
#include "key_driver.h"
#include "stm32f10x_rcc.h"
#include "oled_date.h"
#include "oled_driver.h"
#include "ldr_driver.h"
#include "relay_driver.h"
#include "led_driver.h"
#include "beep_driver.h"
#include "dht11_driver.h"
#include "mq135_driver.h"
#include "ir_driver.h"
#include "led_driver.h"

extern bool alarm_on_ir;
extern bool alarm_on_mq135;
extern bool alarm_on_temp;
extern bool g_threshold_updated;

void bsp_init(void);
void dht11_work(uint8_t *data);
void ldr_work(void);
void mq135_work(void);
void ir_work();
void alarm_work(void);
void oled_init_work(void);
void oled_static_work(void);
void oled_static_menu_work(void);
void oled_show_stage_info(void);

#endif // WORKQUEUE_H
