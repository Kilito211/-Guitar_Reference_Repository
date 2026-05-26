#ifndef OLED_H
#define OLED_H

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define OLED_8X16 8
#define OLED_6X8 6

#define OLED_UNFILLED 0
#define OLED_FILLED 1

#define OLED_I2C_ADDR 0x78

// 初始化函数
void oled_init(void);

// 更新函数
void oled_update(void);
void oled_update_area(int16_t x, int16_t y, uint8_t width, uint8_t height);

// 显示控制函数
void oled_clear(void);
void oled_clear_area(int16_t x, int16_t y, uint8_t width, uint8_t height);
void oled_reverse(void);
void oled_reverse_area(int16_t x, int16_t y, uint8_t width, uint8_t height);

// 显示函数
void oled_show_char(int16_t x, int16_t y, char buf, uint8_t font_size);
void oled_show_string(int16_t x, int16_t y, char *string, uint8_t font_size);
void oled_show_num(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size);
void oled_show_signed_num(int16_t x, int16_t y, int32_t number, uint8_t length, uint8_t font_size);
void oled_show_hex_num(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size);
void oled_show_bin_num(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size);
void oled_show_float_num(int16_t x, int16_t y, double number, uint8_t int_length, uint8_t fra_length, uint8_t font_size);
void oled_show_chinese(int16_t x, int16_t y, char *chinese);
void oled_show_image(int16_t x, int16_t y, uint8_t width, uint8_t height, const uint8_t *image);
void oled_printf(int16_t x, int16_t y, uint8_t font_size, char *format, ...);

// 绘图函数
void oled_draw_point(int16_t x, int16_t y);
uint8_t oled_get_point(int16_t x, int16_t y);
void oled_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void oled_draw_rectangle(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t is_filled);
void oled_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t is_filled);
void oled_dram_circle(int16_t x, int16_t y, uint8_t radius, uint8_t is_filled);
void oled_draw_ellipse(int16_t x, int16_t y, uint8_t A, uint8_t B, uint8_t is_filled);
void oled_draw_arc(int16_t x, int16_t y, uint8_t radius, int16_t start_angle, int16_t end_angle, uint8_t is_filled);

#endif
