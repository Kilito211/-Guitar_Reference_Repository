/**
 * @file main.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief  主函数所在文件
 * @version 0.1
 * @date 2025-12-05
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <stdio.h>
#include "workqueue.h"

int main()
{
	bsp_init(); // 板级初始化
	printf("SYS INIT DONE 1\r\n");
	oled_init_work();	 // OLED初始化
	timer_delay_ms(200); // 等待OLED稳定
	oled_static_work();	 // OLED显示静态界面，即显示不变内容，后续只对变动内容进行更新
	printf("SYS INIT DONE 1\r\n");
	while (1)
	{
		printf("SYS INIT DONE 3\r\n");
		if (g_threshold_updated)
		{
			g_threshold_updated = false;
			oled_static_menu_work();
			oled_threshold_work();
			oled_update();
			printf("Threshold updated via Bluetooth, screen refreshed\n");
		}

		if (g_view_state == VIEW_MAIN)
		{
			dht11_work(g_dht11_data);
			ldr_work();
			mq135_work();
			ir_work();
			alarm_work();
			timer_delay_ms(50);
		}
		else if (g_view_state == VIEW_MENU)
		{
			oled_static_menu_work();
			oled_threshold_work();
			oled_update();
		}
		else
		{
			oled_static_menu_work();
			oled_new_threshold_work();
			oled_update();
		}
	}

	return 0;
}
