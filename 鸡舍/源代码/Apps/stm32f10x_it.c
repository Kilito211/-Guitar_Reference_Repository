/**
 ******************************************************************************
 * @file    GPIO/IOToggle/stm32f10x_it.c
 * @author  MCD Application Team
 * @version V3.5.0
 * @date    08-April-2011
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and peripherals
 *          interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "uart_driver.h"
#include "stdio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "led_driver.h"
#include "systick_driver.h"
#include "typedef.h"
#include "workqueue.h"

extern uint16_t g_ldr_val;
extern volatile uint32_t g_millis;
extern bool alarm_on_temp;
/** @addtogroup STM32F10x_StdPeriph_Examples
 * @{
 */

/** @addtogroup GPIO_IOToggle
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
	/* Go to infinite loop when Memory Manage exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
	/* Go to infinite loop when Bus Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
	/* Go to infinite loop when Usage Fault exception occurs */
	while (1)
	{
	}
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  This function handles PendSV_Handler exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
	// 毫秒计时，用于最小继电器切换间隔等
	g_millis++;
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

/**
 * @}
 */

/**
 * @}
 */

void TIM2_IRQHandler(void)
{
	// 检查是TIM2更新中断标志
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		// 清除中断标志
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		TIM_Cmd(TIM2, DISABLE); // 停止定时器
		// 检查按键实际状态
		if (g_pending_key == KEY_NONE || g_pending_key >= KEY_NUM)
		{
			g_pending_key = KEY_NONE; // 清空待处理按键
			TIM_Cmd(TIM2, DISABLE);	  // 停止定时器，避免定时器一直运行
			return;
		}
		// 执行业务逻辑
		switch (g_pending_key)
		{
		case KEY_PB4:
			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == RESET)
			{
				printf("KEY1被按下\n");
				if (g_view_state == VIEW_MENU) // 在子页面
				{
					if (g_threshold_idx == 0)
						g_threshold_idx = 3; // 4个选项：0,1,2,3
					else
						g_threshold_idx--;
				}
				else if (g_view_state == VIEW_TEMP_SETTING) // 正在修改温度阈值
				{
					new_threshold.temp_threshold--;
					if (new_threshold.temp_threshold < 0)
						new_threshold.temp_threshold = 0;
					printf("新的 temp threshold: %d\n", new_threshold.temp_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_HUMI_SETTING) // 正在修改湿度阈值
				{
					new_threshold.humi_threshold--;
					if (new_threshold.humi_threshold < 0)
						new_threshold.humi_threshold = 0;
					printf("新的 humi threshold: %d\n", new_threshold.humi_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_LDR_SETTING) // 正在修改光照阈值
				{
					new_threshold.ldr_threshold -= 10;
					if (new_threshold.ldr_threshold < 0)
						new_threshold.ldr_threshold = 0;
					printf("新的 ldr threshold: %d\n", new_threshold.ldr_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_IR_SETTING) // 正在修改火焰阈值
				{
					new_threshold.ir_threshold -= 10;
					if (new_threshold.ir_threshold < 0)
						new_threshold.ir_threshold = 0;
					printf("新的 ir threshold: %d\n", new_threshold.ir_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_MQ135_SETTING) // 正在修改烟雾浓度阈值
				{
					new_threshold.mq135_threshold -= 0.5;
					if (new_threshold.mq135_threshold < 0)
						new_threshold.mq135_threshold = 0;
					printf("新的 mq135 threshold: %.2f\n", new_threshold.mq135_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
			}
			break;
		case KEY_PB5:
			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5) == RESET)
			{
				printf("KEY2被按下\n");
				if (g_view_state == VIEW_MENU) // 在子页面
				{
					if (g_threshold_idx >= 3)
						g_threshold_idx = 0;
					else
						g_threshold_idx++;
					// 光标位置高亮
				}
				else if (g_view_state == VIEW_TEMP_SETTING) // 正在修改温度阈值
				{
					// 修改温度阈值
					new_threshold.temp_threshold++;
					printf("新的 temp threshold: %d\n", new_threshold.temp_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_HUMI_SETTING) // 正在修改湿度阈值
				{
					// 修改湿度阈值
					new_threshold.humi_threshold++;
					printf("新的 humi threshold: %d\n", new_threshold.humi_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_LDR_SETTING) // 正在修改光照阈值
				{
					// 修改光照阈值
					new_threshold.ldr_threshold += 10;
					printf("新的 ldr threshold: %d\n", new_threshold.ldr_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_IR_SETTING) // 正在修改火焰阈值
				{
					// 修改火焰阈值
					new_threshold.ir_threshold += 10;
					printf("新的 ir threshold: %d\n", new_threshold.ir_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_MQ135_SETTING) // 正在修改烟雾浓度阈值
				{
					// 修改烟雾浓度阈值
					new_threshold.mq135_threshold += 0.5;
					printf("新的 mq135 threshold: %.2f\n", new_threshold.mq135_threshold);
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
			}
			break;
		case KEY_PB6:
			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == RESET)
			{
				printf("KEY3被按下\n");
				if (g_view_state == VIEW_MENU) // 在子页面 加载到主页
				{
					g_view_state = VIEW_MAIN;
					// 加载主页数据
					oled_static_work();
					oled_update();
					// 主页数据加载完成
				}
				else if (g_view_state == VIEW_TEMP_SETTING) // 正在修改温度阈值 退出修改并恢复为原来的值
				{
					g_view_state = VIEW_MENU;
					new_threshold.temp_threshold = g_threshold.temp_threshold; // 恢复修改用变量的值
																			   // 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_HUMI_SETTING) // 正在修改湿度阈值 退出修改并恢复为原来的值
				{
					g_view_state = VIEW_MENU;
					new_threshold.humi_threshold = g_threshold.humi_threshold; // 恢复修改用变量的值
																			   // 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_LDR_SETTING) // 正在修改光照阈值 退出修改并恢复为原来的值
				{
					g_view_state = VIEW_MENU;
					new_threshold.ldr_threshold = g_threshold.ldr_threshold; // 恢复修改用变量的值
																			 // 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_IR_SETTING) // 正在修改火焰阈值 退出修改并恢复为原来的值
				{
					g_view_state = VIEW_MENU;
					new_threshold.ir_threshold = g_threshold.ir_threshold; // 恢复修改用变量的值
																		   // 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
				else if (g_view_state == VIEW_MQ135_SETTING) // 正在修改烟雾浓度阈值 退出修改并恢复为原来的值
				{
					g_view_state = VIEW_MENU;
					new_threshold.mq135_threshold = g_threshold.mq135_threshold;
					// 主循环会统一刷新显示，避免在中断中直接更新屏幕
				}
			}

				// 如果在主页按下PB6 则尝试复位报警	nb
				if (g_view_state == VIEW_MAIN)
				{
					// 复位所有报警标志并关闭声光报警与继电器（由主循环判断是否要再次开启）
					alarm_on_temp = false;
					alarm_on_ir = false;
					alarm_on_mq135 = false;
					beep_off();
					led_off();
					relay_off();
					printf("报警已复位\n");
				}
			break;
		case KEY_PB12:
			if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == RESET)
			{
				printf("KEY4被按下\n");
				if (g_view_state == VIEW_MENU) // 在子页面 进入修改值的页面
				{
					g_view_state = g_threshold_idx + VIEW_TEMP_SETTING; // 从VIEW_TEMP_SETTING开始依次是温度 湿度 烟雾浓度
				}
				else if (g_view_state == VIEW_TEMP_SETTING) // 正在修改温度阈值 确认修改并保存到全局变量
				{
					g_threshold.temp_threshold = new_threshold.temp_threshold;
					g_view_state = VIEW_MENU;
				}
				else if (g_view_state == VIEW_HUMI_SETTING) // 正在修改湿度阈值 确认修改并保存到全局变量
				{
					g_threshold.humi_threshold = new_threshold.humi_threshold;
					g_view_state = VIEW_MENU;
				}
				else if (g_view_state == VIEW_LDR_SETTING) // 正在修改光照阈值 确认修改并保存到全局变量
				{
					g_threshold.ldr_threshold = new_threshold.ldr_threshold;
					g_view_state = VIEW_MENU;
				}
				else if (g_view_state == VIEW_IR_SETTING) // 正在修改火焰阈值 确认修改并保存到全局变量
				{
					g_threshold.ir_threshold = new_threshold.ir_threshold;
					g_view_state = VIEW_MENU;
				}
				else if (g_view_state == VIEW_MQ135_SETTING) // 正在修改烟雾浓度阈值 确认修改并保存到全局变量
				{
					g_threshold.mq135_threshold = new_threshold.mq135_threshold;
					g_view_state = VIEW_MENU;
				}
				else // 在主页 进入子页面
				{
					g_view_state = VIEW_MENU;
					g_threshold_idx = 0; // 进入子页面默认光标位置
					oled_clear();
					oled_static_menu_work();
					oled_threshold_work();
					oled_update();
				}
			}
			break;
		default:
			break;
		}

		// 清除中断标志
		g_pending_key = KEY_NONE; // 清空待处理按键
	}
}

void EXTI4_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line4) != RESET)
	{
		g_pending_key = KEY_PB4;			// 标记待处理按键为PB4
		TIM_SetCounter(TIM2, 0);			// 定时器计数器清零
		TIM_Cmd(TIM2, ENABLE);				// 启动消抖定时器
		EXTI_ClearITPendingBit(EXTI_Line4); // 清除外部中断标志
	}
}
void EXTI9_5_IRQHandler(void)
{
	// PB5触发
	if (EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		g_pending_key = KEY_PB5;
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	// PB6触发
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		g_pending_key = KEY_PB6;
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
}
void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		g_pending_key = KEY_PB12;
		TIM_SetCounter(TIM2, 0);
		TIM_Cmd(TIM2, ENABLE);
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}

void USART1_IRQHandler(void)
{
	// 判断是否是 RXNE 中断
	// ITStatus USART_GetITStatus(USART_TypeDef* USARTx, uint16_t USART_IT);
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		// 将 DR 的值存入接收缓冲区
		usart1_recv_buf[usart1_recv_index++] = USART_ReceiveData(USART1);
		printf("%c", usart1_recv_buf[usart1_recv_index - 1]);
		if (usart1_recv_index >= USART1_RECV_BUF_SIZE)
		{ // 越界了
			usart1_recv_index = usart1_get_index = usart1_idle_flag = 0;
		}
	}
	else if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET)
	{
		// IDLE 中断标志不能手动清除，必须执行读 SR 再读 DR 这样的读操作才能清除
		// 必须读一次 DR 寄存器，才能清除 IDLE 中断标志
		(void)USART1->DR;
		usart1_idle_flag = 1;
	}
}

void USART2_IRQHandler(void)
{
}

// void USART3_IRQHandler(void)
// {
// 	printf("进入3中断\n		");
// 	int snr_data;
// 	//	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET) {
// 	//		// 读取接收到的字符
// 	//		uint8_t data = USART_ReceiveData(USART3);
// 	//
// 	//		// 存入接收缓冲区
// 	//		if (usart3_recv_index < USART3_RECV_BUF_SIZE - 1) {
// 	//			usart3_recv_buf[usart3_recv_index++] = data;
// 	//		}
// 	//	}

// 	//	else
// 	snr_data = USART_ReceiveData(USART3);
// 	if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET)
// 	{
// 		// 清除 IDLE 标志
// 		volatile uint32_t temp;
// 		temp = USART3->SR;
// 		temp = USART3->DR;
// 		(void)temp;
// 		printf("进入中断3处理\n");
// 		// 添加字符串结束符，便于使用 strcmp

// 		printf("收到蓝牙数据：%d\n", snr_data); // 从 USART1 输出

// 		// 判断是否是 "01" 或 "02"
// 		if (snr_data == 0x01)
// 		{
// 			Bright_val = 100;
// 			Bright_mod = 0;
// 			printf("语音命令：开灯\n");
// 			Set_Light_Brightness(Bright_val);
// 			OLED_ShowChinese(80, 32, "手动");
// 			OLED_ShowNum(80, 16, Bright_val, 3, OLED_8X16);
// 			OLED_Update();
// 		}
// 		else if (snr_data == 0x02)
// 		{
// 			Bright_val = 0;
// 			Bright_mod = 0;
// 			printf("语音命令：关灯\n");
// 			Set_Light_Brightness(Bright_val);
// 			OLED_ShowChinese(80, 32, "手动");
// 			OLED_ShowNum(80, 16, Bright_val, 3, OLED_8X16);
// 			OLED_Update();
// 		}
// 		else if (snr_data == 0x03)
// 		{
// 			Bright_val += 25;
// 			Bright_mod = 0;
// 			if (Bright_val > 100)
// 				Bright_val = 0;
// 			printf("语音命令：调亮\n");
// 			Set_Light_Brightness(Bright_val);
// 			OLED_ShowChinese(80, 32, "手动");
// 			OLED_ShowNum(80, 16, Bright_val, 3, OLED_8X16);
// 			OLED_Update();
// 		}
// 		else if (snr_data == 0x04)
// 		{
// 			Bright_val -= 25;
// 			Bright_mod = 0;
// 			if (Bright_val < 0)
// 				Bright_val = 100;
// 			printf("语音命令：调暗\n");
// 			Set_Light_Brightness(Bright_val);
// 			OLED_ShowChinese(80, 32, "手动");
// 			OLED_ShowNum(80, 16, 100 - Bright_val, 3, OLED_8X16);
// 			OLED_Update();
// 		}
// 		else if (snr_data == 0x05)
// 		{
// 			printf("语音命令：切换模式\n");
// 			Bright_mod = (Bright_mod + 1) % 2;
// 			if (Bright_mod == 1)
// 				OLED_ShowChinese(80, 32, "自动");
// 			if (Bright_mod == 0)
// 				OLED_ShowChinese(80, 32, "手动");
// 			OLED_Update();
// 		}
// 		else
// 		{
// 			printf("未知蓝牙命令\n");
// 		}

// 		// 清除接收索引
// 		usart3_recv_index = 0;
// 	}
// }

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
