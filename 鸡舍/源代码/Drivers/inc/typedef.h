#ifndef TYPEDEF_H
#define TYPEDEF_H

#include "stm32f10x.h"
#include "stm32f10x.h"
#include <stdbool.h>

typedef enum
{
    KEY_NONE = 0,
    KEY_PB4,
    KEY_PB5,
    KEY_PB6,
    KEY_PB12,
    KEY_NUM,
} key_e;

typedef enum _view_state
{
    VIEW_MAIN,
    VIEW_MENU,
    VIEW_TEMP_SETTING,
    VIEW_HUMI_SETTING,
    VIEW_LDR_SETTING,
    VIEW_IR_SETTING,
    VIEW_MQ135_SETTING,
    VIEW_COUNT,
} view_state_e;

typedef struct threshold
{
    uint8_t temp_threshold; // 仅打开风扇
    uint8_t humi_threshold; // 仅打开风扇
    uint16_t ldr_threshold; // 光照强度阈值 低于这个值打开灯
    uint16_t ir_threshold;  // 声光报警
    float mq135_threshold;  // 声光报警
} threshold_t;

// 增长阶段阈值定义（用于温控：最小/最大）
typedef struct growth_stage
{
    char name[12];
    int8_t temp_min; // 温度下限（摄氏度）
    int8_t temp_max; // 温度上限（摄氏度）
} growth_stage_t;

extern key_e g_pending_key; // 键值
extern view_state_e g_view_state;
extern uint8_t g_dht11_data[2];   // 第一个是湿度第二个是温度
extern uint8_t g_threshold_idx;   // 在子页面中当前光标选中的阈值选项 从温度开始 温度选项索引值为0
extern uint16_t g_ldr_val;        // 光照强度
extern float g_mq135_ppm;         // 烟雾浓度
extern threshold_t g_threshold;   // 阈值结构体
extern threshold_t new_threshold; // 在子页面中修改的阈值设置 只有在按下确认键时才会更新到g_threshold
extern bool alarm_on;             // 报警状态
extern char g_bt_ack_msg[16];     // 待发送的确认消息
extern bool g_threshold_updated;  // 标志：阈值由外部（如蓝牙）更新
// 新增全局变量声明
extern volatile uint32_t g_millis; // 毫秒计数
extern uint8_t g_current_stage;    // 当前生长阶段索引
extern growth_stage_t g_stages[];  // 生长阶段数组（定义在 workqueue.c）
extern uint8_t g_stage_count;      // 生长阶段数量
extern bool alarm_on_temp;         // 温度报警标志
extern volatile uint32_t relay_last_toggle_ms; // 继电器上次切换时间(ms)

// 位带操作
// 具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
// IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr) *((volatile unsigned long *)(addr))
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))

// IO口地址映射
#define GPIOA_ODR_Addr (GPIOA_BASE + 12) // 0x4001080C
#define GPIOB_ODR_Addr (GPIOB_BASE + 12) // 0x40010C0C
#define GPIOC_ODR_Addr (GPIOC_BASE + 12) // 0x4001100C
#define GPIOD_ODR_Addr (GPIOD_BASE + 12) // 0x4001140C
#define GPIOE_ODR_Addr (GPIOE_BASE + 12) // 0x4001180C
#define GPIOF_ODR_Addr (GPIOF_BASE + 12) // 0x40011A0C
#define GPIOG_ODR_Addr (GPIOG_BASE + 12) // 0x40011E0C

#define GPIOA_IDR_Addr (GPIOA_BASE + 8) // 0x40010808
#define GPIOB_IDR_Addr (GPIOB_BASE + 8) // 0x40010C08
#define GPIOC_IDR_Addr (GPIOC_BASE + 8) // 0x40011008
#define GPIOD_IDR_Addr (GPIOD_BASE + 8) // 0x40011408
#define GPIOE_IDR_Addr (GPIOE_BASE + 8) // 0x40011808
#define GPIOF_IDR_Addr (GPIOF_BASE + 8) // 0x40011A08
#define GPIOG_IDR_Addr (GPIOG_BASE + 8) // 0x40011E08

// IO口操作,只对单一的IO口 确保值小于16
#define PAout(n) BIT_ADDR(GPIOA_ODR_Addr, n) // 输出
#define PAin(n) BIT_ADDR(GPIOA_IDR_Addr, n)  // 输入

#define PBout(n) BIT_ADDR(GPIOB_ODR_Addr, n) // 输出
#define PBin(n) BIT_ADDR(GPIOB_IDR_Addr, n)  // 输入

#define PCout(n) BIT_ADDR(GPIOC_ODR_Addr, n) // 输出
#define PCin(n) BIT_ADDR(GPIOC_IDR_Addr, n)  // 输入

#define PDout(n) BIT_ADDR(GPIOD_ODR_Addr, n) // 输出
#define PDin(n) BIT_ADDR(GPIOD_IDR_Addr, n)  // 输入

#define PEout(n) BIT_ADDR(GPIOE_ODR_Addr, n) // 输出
#define PEin(n) BIT_ADDR(GPIOE_IDR_Addr, n)  // 输入

#define PFout(n) BIT_ADDR(GPIOF_ODR_Addr, n) // 输出
#define PFin(n) BIT_ADDR(GPIOF_IDR_Addr, n)  // 输入

#define PGout(n) BIT_ADDR(GPIOG_ODR_Addr, n) // 输出
#define PGin(n) BIT_ADDR(GPIOG_IDR_Addr, n)  // 输入

#endif // TYPEDEF_H
