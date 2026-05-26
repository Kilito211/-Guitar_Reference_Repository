/**
 * @file workqueue.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief 工作队列封装、全局变量
 * @version 0.1
 * @date 2026-02-11
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "workqueue.h"
#include <string.h>

threshold_t g_threshold;
threshold_t new_threshold; // 在子页面中修改的阈值设置 只有在按下确认键时才会更新到g_threshold

const threshold_t G_THRESHOLD = {
    .temp_threshold = 25,
    .humi_threshold = 60,
    .ldr_threshold = 300,
    .ir_threshold = 300,
    .mq135_threshold = 200.0,
};

key_e g_pending_key = KEY_NONE;
view_state_e g_view_state = VIEW_MAIN; // 当前在哪个页面
uint8_t g_dht11_data[2];               // 第一个是湿度第二个是温度
uint8_t g_threshold_idx;               // 在子页面中当前光标选中的阈值选项
uint16_t g_ldr_val = 0;                // 光照强度
uint16_t g_ir_val = 0;                 // 火焰传感器数值
float g_mq135_ppm = 0;                 // 烟雾浓度
bool alarm_on_ir = false;              // 火焰报警状态
bool alarm_on_mq135 = false;           // 烟雾报警状态
char g_bt_ack_msg[16] = "";            // 待发送的确认消息
bool g_threshold_updated = false;      // 标志：阈值由蓝牙或外部更新，需要刷新界面
// 新增全局
volatile uint32_t g_millis = 0; // 毫秒计数
uint8_t g_current_stage = 0;    // 当前生长阶段
growth_stage_t g_stages[4];     // 支持最多4个阶段
uint8_t g_stage_count = 0;
bool alarm_on_temp = false; // 温度导致的报警
// 继电器保护：最小开关间隔（毫秒）和上一次切换时间
const uint32_t RELAY_MIN_TOGGLE_MS = 30000; // 30s 最小切换间隔
volatile uint32_t relay_last_toggle_ms = 0;
/**
 * @brief 板级初始化
 */
void bsp_init(void)
{
    g_threshold = G_THRESHOLD;   // 初始化全局变量
    new_threshold = g_threshold; // 初始化修改用变量

    // 初始化生长阶段（示例4个阶段，可由按键或蓝牙修改）
    g_stage_count = 3;
    strcpy(g_stages[0].name, "育雏");
    g_stages[0].temp_min = 28;
    g_stages[0].temp_max = 32;
    strcpy(g_stages[1].name, "生长");
    g_stages[1].temp_min = 22;
    g_stages[1].temp_max = 26;
    strcpy(g_stages[2].name, "保温");
    g_stages[2].temp_min = 18;
    g_stages[2].temp_max = 22;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    SystemInit();
    usart1_init(115200);
    usart_else_init(9600);
    timer_init(72);
    key_init();
    oled_init();
    ldr_init();
    relay_gpio_init();
    led_init();
    beep_init();
    dht11_init();
    mq135_init();
    printf("SystemCoreClock = %ld Hz\r\n", SystemCoreClock);
}

/**
 * @brief DHT11读取任务
 *
 * @param data 存储数据的地址
 * @brief 每次执行都会读取温湿度数据并显示在oled上 数据存储在全局变量g_dht11_data中 0为湿度 1为温度
 * @return uint8_t 每次读取 成功返回0 失败返回1
 */
void dht11_work(uint8_t *data)
{
    int res;
    res = dht11_read_data(g_dht11_data);
    if (!res)
    {
        // 温度/湿度判断（优先温度按阶段判断）
        int8_t temp = (int8_t)g_dht11_data[1];
        int8_t humi = (int8_t)g_dht11_data[0];

        // 使用生长阶段的上下限判断温度报警
        if (g_stage_count > 0 && g_current_stage < g_stage_count)
        {
            growth_stage_t *st = &g_stages[g_current_stage];
            // 温度低于最小阈值 -> 开启加热继电器
            if (temp < st->temp_min)
            {
                // 滞回：只有当上次切换超过最小间隔时才允许再次切换
                if ((g_millis - relay_last_toggle_ms) > RELAY_MIN_TOGGLE_MS)
                {
                    relay_on();
                    relay_last_toggle_ms = g_millis;
                }
            }
            else if (temp > st->temp_max)
            {
                if ((g_millis - relay_last_toggle_ms) > RELAY_MIN_TOGGLE_MS)
                {
                    relay_off();
                    relay_last_toggle_ms = g_millis;
                }
            }
            // 报警判定：温度严重超限触发声光报警
            if (temp < st->temp_min - 5 || temp > st->temp_max + 5)
                alarm_on_temp = true;
            else
                alarm_on_temp = false;
        }
        else
        {
            // 回退到原有阈值判断（兼容）
            if (g_dht11_data[1] > g_threshold.temp_threshold || g_dht11_data[0] > g_threshold.humi_threshold)
                relay_on();
            else
                relay_off();
        }

        printf("dht11 当前温度:%d℃ 当前湿度:%d%%\n", g_dht11_data[1], g_dht11_data[0]);
        if (g_view_state == VIEW_MAIN)
        {
            oled_show_num(36, 0, g_dht11_data[0], 3, OLED_8X16);
            oled_update();
        }
    }
    else
        printf("DHT11 Read Error\n");

    // oled_show_num(0, 16, 123, 3, OLED_8X16);
}

/**
 * @brief 光敏传感器任务
 *
 * @brief 每次执行都会读取光照强度数据并显示在oled上 数据存储在g_ldr_val中
 */
void ldr_work(void)
{
    g_ldr_val = ldr_lux_data();
    // 判断是否打开灯 低于阈值打开 否则关闭
    if (g_ldr_val < g_threshold.ldr_threshold)
        led_on();
    else
        led_off();

    printf("Light %d lux\n", g_ldr_val);
    oled_clear_area(36, 16, 30, 16);
    if (g_view_state == VIEW_MAIN)
    {
        oled_show_num(36, 16, g_ldr_val, 3, OLED_8X16);
        oled_update();
    }
}

/**
 * @brief 烟雾传感器任务
 *
 * @brief 每次执行会读取mq135数据并显示在屏幕上 数据存储在g_mq135_ppm 用MQ135_MODE宏区分读取模拟值或者数字值
 */
void mq135_work(void)
{
#if MQ135_MODE
    g_mq135_ppm = mq135_get_data_ppm();
    if (g_mq135_ppm > g_threshold.mq135_threshold)
        alarm_on_mq135 = true;
    else
        alarm_on_mq135 = false;

    printf("MQ-135 %.2f ppm", g_mq135_ppm);
    oled_clear_area(68, 32, 30, 16);
    if (g_view_state == VIEW_MAIN)
    {
        oled_show_num(68, 32, g_mq135_ppm, 3, OLED_8X16);
        oled_update();
    }
#else g_mq135_ppm = mq135_get_data_();
#endif
}

/**
 * @brief 火焰传感器任务
 *
 * @brief 每次执行会读取火焰传感器数据 存储在g_ir_val中 用宏区分 但是模拟模式没写
 */
void ir_work(void) // 加上 void 消除警告
{
    printf("ir_work_init\n");
#if IR_MODE
    char buf[16]; // 分配 16 字节的本地空间
    g_ir_val = ir_fire_data();

    if (g_ir_val > g_threshold.ir_threshold)
        alarm_on_ir = true;
    else
        alarm_on_ir = false;

    sprintf(buf, "%d", g_ir_val); // 现在安全了
    printf("IR %s\n", buf);       // 打印字符串
    if (g_view_state == VIEW_MAIN)
    {
        oled_show_num(68, 32, g_ldr_val, 3, OLED_8X16);
        oled_update();
    }
    printf("ir_work_end\n");
#endif
}

/**
 * @brief 声光报警任务
 */
void alarm_work(void)
{
    printf("alarm_work_init\n");
    bool alarm_on = alarm_on_ir || alarm_on_mq135 || alarm_on_temp; // 只要有一个报警条件满足就报警
    if (alarm_on)
    {
        beep_reverse(); // 蜂鸣器反转
        led_reverse();  // 警报灯闪烁
        relay_on();     // 报警时打开风扇
        if (alarm_on_mq135)
            timer_delay_ms(200); // 火焰报警闪烁频率较快
    }
    else
    {
        beep_off();  // 没有报警条件时关闭蜂鸣器
        led_off();   // 关闭报警灯
        relay_off(); // 没有报警条件时关闭风扇
    }
}

/**
 * @brief 开机界面oled 中心显示食品储藏系统
 */
void oled_init_work(void)
{
    oled_clear();

    // 食品储藏
    oled_show_chinese(32, 24, "食品储藏");
    oled_update();
}

/**
 * @brief 静态不需要刷新的oled部分
 */
void oled_static_work(void)
{
    oled_clear();

    // 温度
    oled_show_chinese(0, 0, "温度");
    oled_show_char(34, 0, ':', OLED_8X16);
    // 湿度
    oled_show_chinese(64, 0, "湿度");
    oled_show_char(98, 0, ':', OLED_8X16);

    oled_show_chinese(0, 16, "光强");
    oled_show_char(34, 16, ':', OLED_8X16);
    oled_show_chinese(64, 16, "火焰");
    oled_show_char(98, 16, ':', OLED_8X16);
    // 烟雾浓度
    oled_show_chinese(0, 32, "烟雾浓度");
    oled_show_char(66, 32, ':', OLED_8X16);
    //  x动模式
    oled_show_chinese(0, 48, "点击确定进入设置");

    oled_update();
    printf("OLED static work done\n");
    oled_show_stage_info();
}

/**
 * @brief 在屏幕上显示当前生长阶段信息和系统状态（局部刷新）
 */
void oled_show_stage_info(void)
{
    // 在右上角显示阶段名和温度区间，右下显示 HEAT/ALARM 指示
    // 清理右侧区域（x:96-127, y:0-31）
    oled_clear_area(96, 0, 32, 32);

    // 阶段名
    if (g_stage_count > 0 && g_current_stage < g_stage_count)
    {
        const char *name = g_stages[g_current_stage].name;
        oled_show_string(98, 0, (char *)name, OLED_6X8);

        // 显示 Tmin/Tmax
        char buf[16];
        sprintf(buf, "T:%d~%d", g_stages[g_current_stage].temp_min, g_stages[g_current_stage].temp_max);
        oled_show_string(98, 10, buf, OLED_6X8);
    }
    else
    {
        oled_show_string(98, 0, "Stage:--", OLED_6X8);
        oled_show_string(98, 10, "T:--~--", OLED_6X8);
    }

    // HEAT 指示（继电器状态）
    // 假设继电器打开时 relay_on() 会让 RELAY_SWITCH_ON 宏生效，使用读取宏不可行（无读宏），
    // 这里我们只根据 relay_last_toggle_ms 和 g_millis 简单判断：如果最近30s内有打开动作，则显示 ON（近似）
    if ((int32_t)(g_millis - relay_last_toggle_ms) < (int32_t)RELAY_MIN_TOGGLE_MS && relay_last_toggle_ms != 0)
        oled_show_string(98, 20, "HEAT:ON", OLED_6X8);
    else
        oled_show_string(98, 20, "HEAT:OFF", OLED_6X8);

    // ALARM 指示（任一报警）
    if (alarm_on_ir || alarm_on_mq135 || alarm_on_temp)
        oled_show_string(116, 20, "ALARM", OLED_6X8);
    else
        oled_show_string(116, 20, "      ", OLED_6X8);

    oled_update_area(96, 0, 32, 32);
}

static void oled_draw_circle_marker(int16_t x, int16_t y, bool filled)
{
    if (filled)
    {
        for (int16_t dy = -1; dy <= 1; dy++)
            for (int16_t dx = -1; dx <= 1; dx++)
                oled_draw_point(x + dx, y + dy);
    }
    else
    {
        oled_draw_point(x - 2, y);
        oled_draw_point(x + 2, y);
        oled_draw_point(x, y - 2);
        oled_draw_point(x, y + 2);
        oled_draw_point(x - 1, y - 1);
        oled_draw_point(x - 1, y + 1);
        oled_draw_point(x + 1, y - 1);
        oled_draw_point(x + 1, y + 1);
    }
}

static void oled_draw_threshold_marker(void)
{
    // 子页面只有四个可见阈值项：温度、湿度、光照、火焰
    if (g_view_state != VIEW_MENU && (g_view_state < VIEW_TEMP_SETTING || g_view_state > VIEW_IR_SETTING))
        return;

    uint8_t marker_idx;
    bool filled = false;

    if (g_view_state == VIEW_MENU)
    {
        marker_idx = g_threshold_idx;
        filled = false;
    }
    else
    {
        marker_idx = g_view_state - VIEW_TEMP_SETTING;
        filled = true;
    }

    if (marker_idx >= 4)
        return;

    const int16_t marker_x = 120;
    const int16_t marker_y = 8 + marker_idx * 16;
    oled_draw_circle_marker(marker_x, marker_y, filled);
}

void oled_static_menu_work(void)
{
    oled_clear();

    // 温度阈值
    oled_show_chinese(0, 0, "温度阈值");
    oled_show_char(68, 0, ':', OLED_8X16);
    // 湿度阈值
    oled_show_chinese(0, 16, "湿度阈值");
    oled_show_char(68, 16, ':', OLED_8X16);
    // 光照阈值
    oled_show_chinese(0, 32, "光照阈值");
    oled_show_char(68, 32, ':', OLED_8X16);
    // 火焰阈值
    oled_show_chinese(0, 48, "火焰阈值");
    oled_show_char(68, 48, ':', OLED_8X16);
    // // 烟雾浓度阈值
    // oled_show_chinese(64, 0, "烟雾浓度阈值");
    // oled_show_char(98, 0, ':', OLED_8X16);

    oled_draw_threshold_marker();
}

void oled_threshold_work(void)
{
    oled_show_num(80, 0, g_threshold.temp_threshold, 3, OLED_8X16);
    oled_show_num(80, 16, g_threshold.humi_threshold, 3, OLED_8X16);
    // 更新阶段和系统状态显示区域
    oled_show_stage_info();
    oled_show_num(80, 32, g_threshold.ldr_threshold, 3, OLED_8X16);
    oled_show_num(80, 48, g_threshold.ir_threshold, 3, OLED_8X16);
}

void oled_new_threshold_work(void)
{
    oled_show_num(80, 0, new_threshold.temp_threshold, 3, OLED_8X16);
    oled_show_num(80, 16, new_threshold.humi_threshold, 3, OLED_8X16);
    oled_show_num(80, 32, new_threshold.ldr_threshold, 3, OLED_8X16);
    oled_show_num(80, 48, new_threshold.ir_threshold, 3, OLED_8X16);
}