/**
 * @file oled.c
 * @author kilito(kilito.hyx@gmail.com)
 * @brief   oled屏幕驱动
 * @version 0.1
 * @date 2026-01-08
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "stm32f10x.h"
#include "oled_driver.h"
#include "oled_date.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

/******* 全局变量 *******/

uint8_t oled_display_buf[8][128]; // oled显存数组

/******* 引脚配置 *******/

/**
 * @brief OLED写SCL高低电平
 * @param bit_value 要写入SCL的电平值 0/1
 * @details 当上层函数需要写SCL时，此函数会被调用
 *          用户需要根据参数传入的值，将SCL置为高电平或者低电平
 *          当参数传入0时，置SCL为低电平，当参数传入1时，置SCL为高电平
 */
void oled_w_scl(uint8_t bit_value)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)bit_value);
}

/**
 * @brief OLED写SDA高低电平
 * @param bit_value 要写入SDA的电平值 0/1
 * @details 当上层函数需要初始化时，此函数会被调用
 *          用户需要将SCL和SDA引脚初始化为开漏模式，并释放引脚
 */
void oled_w_sda(uint8_t bit_value)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)bit_value);
}

/**
 * @brief oled引脚初始化
 * @details 当上层函数需要初始化时，此函数会被调用
 *          用户需要将SCL和SDA引脚初始化为开漏模式，并释放引脚
 */
void oled_gpio_init(void)
{
    uint32_t i, j;

    // 延时等待OLED供电稳定
    for (i = 0; i < 1000; i++)
    {
        for (j = 0; j < 1000; j++)
            ;
    }

    // 使能APB2总线
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // 配置引脚
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 释放SCL和SDA
    oled_w_scl(1);
    oled_w_sda(1);
}

/******* 通信协议 *******/

/**
 * @brief 发送I2C起始信号
 */
void oled_i2c_start(void)
{
    oled_w_sda(1); // 释放SDA，确保SDA为高电平
    oled_w_scl(1); // 释放SCL，确保SCL为高电平
    oled_w_sda(0); // 在SCL高电平期间，拉低SDA，产生起始信号
    oled_w_scl(0); // 发送起始信号后把SCL也拉低，占用总线，方便时许拼接
}

/**
 * @brief 发送I2C终止信号
 */
void oled_i2c_stop(void)
{
    oled_w_sda(0); // 拉低SDA，确保SDA为低电平
    oled_w_scl(1); // 释放SCL，使SCL呈现高电平
    oled_w_sda(1); // 在SCL高电平期间，释放SDA，产生终止信号
}

/**
 * @brief I2C发送一个字节
 * @param byte 要发送的字节数据 范围0x00~0xFF
 */
void oled_i2c_send_byte(uint8_t byte)
{
    int i;
    // 循环8次，每次发送以为数据
    for (i = 0; i < 8; i++)
    {
        // 使用掩码方式去除byte的指定以为数据并写入到SDA线
        oled_w_sda(!!(byte & (0x80 >> i)));
        oled_w_scl(1); // 释放SCL，从机在SCL高电平几千读取SDA
        oled_w_scl(0); // 拉低SDL，主机开始发送下一位数据
    }
    oled_w_scl(1); // 额外的一个始终，不处理应答信号
    oled_w_scl(0);
}

/**
 * @brief oled写命令
 * @param command 要写入的命令值 范围0x00~0xFF
 */
void oled_write_command(uint8_t command)
{
    oled_i2c_start();                  // 发送起始信号
    oled_i2c_send_byte(OLED_I2C_ADDR); // 发送oled的i2c从机地址
    oled_i2c_send_byte(0x00);
    oled_i2c_send_byte(command); // 写入指令
    oled_i2c_stop();             // 发送终止信号
}

/**
 * @brief oled写数据
 * @param date 要写入的数据的指针
 * @param cont 要写输入据的数量
 */
void oled_write_date(uint8_t *date, uint8_t cont)
{
    int i;
    oled_i2c_start();                  // 发送起始信号
    oled_i2c_send_byte(OLED_I2C_ADDR); // 发送OLED的I2C从机地址
    oled_i2c_send_byte(0x40);          // 控制字节，给0x40，表示即将写数据

    // 循环cont次，进行连续数据写入
    for (i = 0; i < cont; i++)
        oled_i2c_send_byte(date[i]);
    oled_i2c_stop(); // i2c终止
}

/******* 硬件配置 *******/

/**
 * @brief oled初始化
 * @details 使用oled前先调用此初始化函数
 */
void oled_init(void)
{
    oled_gpio_init(); // 初始化GPIO

    // 写入oled配置命令
    oled_write_command(0xAE); // 设置显示开启/关闭，0xAE关闭 0xAF开启

    oled_write_command(0xD5); // 设置显示始终分频比/振荡器频率
    oled_write_command(0x80); // 0x00~0xFF

    oled_write_command(0xA8); // 设置多路复用率
    oled_write_command(0x3F); // 0x0E~0x3F

    oled_write_command(0xD3); // 设置显示偏移
    oled_write_command(0x00); // 0x00~0x7F

    oled_write_command(0x40); // 设置显示开始行，0x40~0x7F

    oled_write_command(0xA1); // 设置左右方向，0xA1正常 0xA0左右镜像

    oled_write_command(0xC8); // 设置上下方向，0XC8正常 0xC0上下镜像

    oled_write_command(0xDA); // 设置COM引脚硬件配置
    oled_write_command(0x12);

    oled_write_command(0x81); // 设置对比度
    oled_write_command(0xCF); // 0x00~0xFF

    oled_write_command(0xD9); // 设置预充电周期
    oled_write_command(0xF1);

    oled_write_command(0xDB); // 设置VCOMH取消选择级别
    oled_write_command(0x30);

    oled_write_command(0xA4); // 设置整个显示打开/关闭

    oled_write_command(0xA6); // 设置正常/反色显示，0xA6正常 0xA7反色

    oled_write_command(0x8D); // 设置充电泵
    oled_write_command(0x14);

    oled_write_command(0xAF); // 开启显示

    oled_clear();  // 清空现存数组
    oled_update(); // 更新缓冲区
}

/**
 * @brief oled设置显示光标位置
 * @param page 光标所在页 0~7
 * @param x 光标所在x轴坐标 0~127
 * @details oled默认的y轴，只能8个bit为一组写入，即一页等于8行，共8页
 */
void oled_set_cursor(uint8_t page, uint8_t x)
{
    oled_write_command(0xB0 | page);              // 设置页位置
    oled_write_command(0x10 | ((x & 0xF0) >> 4)); // 设置X位置高4位
    oled_write_command(0x00 | (x & 0x0F));        // 设置X位置低4位
}

/******* 工具函数 *******/

/**
 * @brief 次方函数
 *
 * @param x 底数
 * @param y 指数
 * @return uint32_t 等于X的Y次方
 */
uint32_t oled_pow(uint32_t x, uint32_t y)
{
    uint32_t result = 1; // 结果默认为1
    while (y--)          // 类乘y次方
        result *= x;
    return result;
}

/**
 * @brief 判断指定点是否在指定多边形内部
 *
 * @param nvert 多边形的定点数
 * @param vertx 包含多边形顶点的x坐标的数组
 * @param verty 包含多边形顶点的y坐标的数组
 * @param testx 测试点的x坐标
 * @param testy 测试点的y坐标
 * @return uint8_t 指定点是否在指定多边形内部，1:在内部 0:不在内部
 */
uint8_t oled_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
    int16_t i, j, c = 0;

    for (i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        if (((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
        {
            c = !c;
        }
    }
    return c;
}

/**
 * @brief 判断指定点是否在指定角度内
 *
 * @param x 指定点的x坐标
 * @param y 指定点的y坐标
 * @param start_angle 起始角度，范围-180~180
 * @param end_angle 终止角度，范围-180~180
 * @details 水平向右位0度，水平向左位180度或-180度，下方为正数，上方为附属，顺时针旋转
 * @return uint8_t 指定点是否在指定角度内不 1:在内部 0:不在内部
 */
uint8_t oled_is_in_angle(int16_t x, int16_t y, int16_t start_angle, int16_t end_angle)
{
    int16_t point_angle;
    point_angle = atan2(y, x) / 3.14 * 180; // 计算指定点的弧度，并转换为角度表示
    if (start_angle < end_angle)            // 起始角度小于终止角度的情况
    {
        /*如果指定角度在起始终止角度之间，则判定指定点在指定角度*/
        if (point_angle >= start_angle && point_angle <= end_angle)
        {
            return 1;
        }
    }
    else // 起始角度大于于终止角度的情况
    {
        /*如果指定角度大于起始角度或者小于终止角度，则判定指定点在指定角度*/
        if (point_angle >= start_angle || point_angle <= end_angle)
        {
            return 1;
        }
    }
    return 0; // 不满足以上条件，则判断判定指定点不在指定角度
}

/******* 功能函数 *******/

/**
 * @brief 将oled现存数组更新到oled屏幕
 * @details 所有的显示函数，都只是对OLED显存数组进行读写
 *           随后调用OLED_Update函数或OLED_UpdateArea函数
 *           才会将显存数组的数据发送到OLED硬件，进行显示
 *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_update(void)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        oled_set_cursor(i, 0);                     // 设置光标在当前页的第一列
        oled_write_date(oled_display_buf[i], 128); // 连续写入128个数据，将当前页填满
    }
}

/**
 * @brief 将oled现存数组部分更新到oled屏幕
 *
 * @param x 指定区域左上角的横坐标 范围-32768~32767 ,屏幕区域：0~127
 * @param y 指定区域左上角的纵坐标，范围： - 32768 ~32767，屏幕区域：0 ~63
 * @param width 指定区域的宽度，范围：0 ~128
 * @param height 指定区域的高度，范围：0 ~64
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_update_area(int16_t x, int16_t y, uint8_t width, uint8_t height)
{
    int16_t page, page1;
    int i;

    // 负数坐标在计算页地址时需要加一个偏移
    //( y + height - 1) / 8 + 1的目的是(y + height) / 8并向上取整
    page = y / 8;
    page1 = (y + height - 1) / 8 + 1;

    if (y < 0)
    {
        page -= 1;
        page1 -= 1;
    }

    // 遍历指定区域涉及的相关页
    for (i = page; i < page1; i++)
    {
        if (x >= 0 && x <= 127 && i >= 0 && i <= 7) // 超出屏幕的内容不显示
        {
            // 设置光标位置为相关页的指定列
            oled_set_cursor(i, x);
            // 连续写入width个数据，将现存数组的数据写入到oled硬件
            oled_write_date(&oled_display_buf[i][x], width);
        }
    }
}

/**
 * @brief 将OLED显存数组全部清零
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_clear(void)
{
    int i, j;
    for (i = 0; i < 8; i++)                // 遍历8页
        for (j = 0; j < 128; j++)          // 遍历128列
            oled_display_buf[i][j] = 0x00; // 将现存数组数据全部清零
}

/**
 * @brief 将OLED显存数组部分清零
 * @param x 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param width  指定区域的宽度，范围：0~128
 * @param height 指定区域的高度，范围：0~64
 */
void oled_clear_area(int16_t x, int16_t y, uint8_t width, uint8_t height)
{
    int i, j;
    for (i = y; i < y + height; i++)                              // 遍历指定页
        for (j = x; j < x + width; j++)                           // 遍历指定类
            if (j >= 0 && j <= 127 && i >= 0 && i <= 63)          // 超出屏幕的内容不显示
                oled_display_buf[i / 8][j] &= ~(0x01 << (i % 8)); // 将显存数组指定数据清零
}

/**
 * @brief 将OLED显存数组全部取反
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_reverse(void)
{
    int i, j;
    for (i = 0; i < 8; i++) // 遍历八页
    {
        for (j = 0; j < 128; j++)           // 遍历128列
            oled_display_buf[i][j] ^= 0xFF; // 将显存数组数据全部取反
    }
}

/**
 * @brief 将OLED显存数组部分取反
 *
 * @param x X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param width  Width 指定区域的宽度，范围：0~128
 * @param height Height 指定区域的高度，范围：0~64
 */
void oled_reverse_area(int16_t x, int16_t y, uint8_t width, uint8_t height)
{
    int i, j;
    for (i = y; i < y + height; i++)                           // 遍历指定页
        for (j = x; j < x + width; j++)                        // 遍历指定列
            if (j >= 0 && j <= 127 && i >= 0 && i <= 63)       // 超出屏幕范围不显示
                oled_display_buf[i / 8][j] ^= 0x01 << (i % 8); // 将显存数组指定数据取反
}

/**
 * @brief OLED显示一个字符
 * @param x    指定字符左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y    指定字符左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param buf  指定要显示的字符，范围：ASCII码可见字符
 * @param font_size FontSize 指定字体大小
 *           范围：OLED_8X16		宽8像素，高16像素
 *                 OLED_6X8		宽6像素，高8像素
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_char(int16_t x, int16_t y, char buf, uint8_t font_size)
{
    if (font_size == OLED_8X16)
        oled_show_image(x, y, 8, 16, OLED_F8x16[buf - ' ']);
    else if (font_size == OLED_6X8)
        oled_show_image(x, y, 6, 8, OLED_F6x8[buf - ' ']);
}

/**
 * @brief OLED显示字符串
 * @param x 指定字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param string     指定要显示的字符串，范围：ASCII码可见字符组成的字符串
 * @param font_size  指定字体大小
 *                      范围：OLED_8X16		宽8像素，高16像素
 *                            OLED_6X8		宽6像素，高8像素
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_string(int16_t x, int16_t y, char *string, uint8_t font_size)
{
    int i;
    for (i = 0; string[i] != '\0'; i++)                             // 遍历字符串的字符
        oled_show_char(x + i * font_size, y, string[i], font_size); // 调用oled_show_char函数，依次显示每个字符
}

/**
 * @brief OLED显示数字（十进制，正整数）
 * @param x 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param number 指定要显示的数字，范围：0~4294967295
 * @param length 指定数字的长度，范围：0~10
 * @param font_size 指定字体大小
 *                  范围：OLED_8X16		宽8像素，高16像素
 *                  OLED_6X8		宽6像素，高8像素
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_num(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size)
{
    int i;
    for (i = 0; i < length; i++)                                                                           // 遍历数字的每一位
        oled_show_char(x + i * font_size, y, number / oled_pow(10, length - i - 1) % 10 + '0', font_size); // 将数字参数转化为字符，调用显示字符函数显示数字
}

/**
 * @brief oled显示有符号数字(十进制，正数)
 * @param x 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param number 指定要显示的数字，范围：-2147483648~2147483647
 * @param length 指定数字的长度，范围：0~10
 * @param font_size FontSize 指定字体大小
 *                  范围：OLED_8X16		宽8像素，高16像素
 *                       OLED_6X8		宽6像素，高8像素
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_signed_num(int16_t x, int16_t y, int32_t number, uint8_t length, uint8_t font_size)
{
    uint32_t number1;
    int i;
    if (number >= 0) // 数字大于等于0
    {
        oled_show_char(x, y, '+', font_size); // 显示+号
        number1 = number;                     // number1直接等于number
    }
    else // 数字小于0
    {
        oled_show_char(x, y, '-', font_size); // 显示一号
        number1 = -number;                    // number1等于number取负
    }

    for (i = 0; i < length; i++) // 遍历数字的每一位
    {
        // 调用oled_show_char函数，依次显示每个数字
        // number1 / oled_pow(10,length - i -1) % 10 可以十进制提取数字的每一位
        // + '0' 可以将数字转化为字符
        oled_show_char(x + (i + 1) * font_size, y, number1 / oled_pow(10, length - i - 1) % 10 + '0', font_size);
    }
}

/**
 * @brief OLED显示十六进制数字（十六进制，正整数）
 * @param x 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
 * @param length 指定数字的长度，范围：0~8
 * @param font_size FontSize 指定字体大小
 *                  范围：OLED_8X16		宽8像素，高16像素
 *                  OLED_6X8		宽6像素，高8像素
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_hex_num(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size)
{
    uint8_t single_number;
    int i;
    for (i = 0; i < length; i++) // 遍历数字的每一位
    {
        single_number = number / oled_pow(16, length - i - 1) % 16; // 以十六进制提取数字每一位

        if (single_number < 10) // 单个数字小于10
        {
            // 调用oled_show_char函数，显示此数字
            // + '0' 可将数字转换为字符格式
            oled_show_char(x + i * font_size, y, single_number + '0', font_size);
        }
        else
        {
            // 调用oled_show_char函数，显示此数字
            // + 'A' 可将数字转换为从A开始的十六进制的字符
            oled_show_char(x + i * font_size, y, single_number - 10 + 'A', font_size);
        }
    }
}

/**
 * @brief OLED显示二进制数字（二进制，正整数）
 * @param x 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
 * @param length 指定数字的长度，范围：0~16
 * @param font_size FontSize 指定字体大小
 *                  范围：OLED_8X16		宽8像素，高16像素
 *                  OLED_6X8		宽6像素，高8像素
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_bin_num(int16_t x, int16_t y, uint32_t number, uint8_t length, uint8_t font_size)
{
    for (int i = 0; i < length; i++) // 遍历数学的每一位
    {
        // 调用oled_show_char函数，显示此数字
        // number / oled_pow(2,length - i - 1) % 2可以二进制提取数字的每一位
        // + ‘0’ 可将数字转换为字符格式
        oled_show_char(x + i * font_size, y, number / oled_pow(2, length - i - 1) % 2 + '0', font_size);
    }
}

/**
 * @brief OLED显示浮点数字（十进制，小数）
 * @param x 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param number 指定要显示的数字，范围：-4294967295.0~4294967295.0
 * @param int_length 指定数字的整数位长度，范围：0~10
 * @param fra_length 指定数字的小数位长度，范围：0~9，小数进行四舍五入显示
 * @param font_size FontSize 指定字体大小
 *                  范围：OLED_8X16		宽8像素，高16像素
 *                  OLED_6X8		宽6像素，高8像素
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_float_num(int16_t x, int16_t y, double number, uint8_t int_length, uint8_t fra_length, uint8_t font_size)
{
    uint32_t pow_num, int_num, fra_num;

    if (number >= 0) // 数字大于等于0
    {
        oled_show_char(x, y, '+', font_size); // 显示+号
    }
    else // 数字小于0
    {
        oled_show_char(x, y, '-', font_size); // 显示一号
        number = -number;                     // number取负
    }

    // 提取整数部分和小数部分
    int_num = number;                   // 直接赋值给整型变量，提取整数
    number -= int_num;                  // 将number的整数减掉，防止之后将小数乘到整数时因数过大造成错误
    pow_num = oled_pow(10, fra_length); // 根据指定小数的位数，确定乘数
    fra_num = round(number * pow_num);  // 将小数乘到整数，同时四舍五入，避免显示误差
    int_num += fra_num / pow_num;       // 若四舍五入造成了进位，则需要再加给整数

    // 显示整数部分
    oled_show_num(x + font_size, y, int_num, int_length, font_size);

    // 显示小数点
    oled_show_char(x + (int_length + 1) * font_size, y, '.', font_size);

    // 显示小数部分
    oled_show_num(x + (int_length + 2) * font_size, y, fra_num, fra_length, font_size);
}

/**
 * @brief OLED显示汉字串
 * @param x 指定汉字串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定汉字串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param chinese 指定要显示的汉字串，范围：必须全部为汉字或者全角字符，不要加入任何半角字符
 *           显示的汉字需要在OLED_Data.c里的OLED_CF16x16数组定义
 *           未找到指定汉字时，会显示默认图形（一个方框，内部一个问号）
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_chinese(int16_t x, int16_t y, char *chinese)
{
    uint8_t p_chinese = 0; // 计数
    uint8_t p_index;
    char single_chinese[OLED_CHN_CHAR_WIDTH + 1] = {0}; // 单个汉字字符串
    int i;

    for (i = 0; chinese[i] != '\0'; i++) // 遍历字符串
    {
        single_chinese[p_chinese] = chinese[i]; // 提取字符串数据到单个汉字
        p_chinese++;                            // 计数自增

        // 当提取次数到达OLED_CHN_CHAR_WIDTH时，即代表提取到了一个完整的汉字
        if (p_chinese >= OLED_CHN_CHAR_WIDTH)
        {
            p_chinese = 0; // 计数归零

            // 遍历字库，寻找匹配的汉字
            for (p_index = 0; strcmp(OLED_CF16x16[p_index].Index, "") != 0; p_index++)
            {
                if (strcmp(OLED_CF16x16[p_index].Index, single_chinese) == 0)
                {
                    break;
                }
            }
            oled_show_image(x + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, y, 16, 16, OLED_CF16x16[p_index].Data);
        }
    }
}

/**
 * @brief OLED显示图像
 * @param x 指定图像左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定图像左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param width  指定图像的宽度，范围：0~128
 * @param height 指定图像的高度，范围：0~64
 * @param image 指定要显示的图像
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_show_image(int16_t x, int16_t y, uint8_t width, uint8_t height, const uint8_t *image)
{
    uint16_t page, shift;
    int i, j;

    // 将图像所在区域清空
    oled_clear_area(x, y, width, height);

    // 遍历指定图像涉及的相关也
    for (i = 0; i < (height - 1) / 8 + 1; i++)
    {
        for (j = 0; j < width; j++)
        {
            if (x + j >= 0 && x + j <= 127) // 超出屏幕的内容不显示
            {
                // 负数坐标在计算页地址和移位时需要加一个偏移
                page = y / 8;
                shift = y % 8;
                if (y < 0)
                {
                    page -= 1;
                    shift += 8;
                }
                if (page + i >= 0 && page + i <= 7) // 超出屏幕的内容不显示
                {
                    oled_display_buf[page + i][x + j] |= image[i * width + j] << (shift);
                }
                if (page + i + 1 >= 0 && page + i + 1 <= 7)
                {
                    oled_display_buf[page + i + 1][x + j] |= image[i * width + j] >> (8 - shift);
                }
            }
        }
    }
}

/**
 * @brief OLED使用printf函数打印格式化字符串
 *
 * @param x 指定格式化字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定格式化字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param font_size FontSize 指定字体大小
 *                  范围：OLED_8X16		宽8像素，高16像素
 *                  OLED_6X8		宽6像素，高8像素
 * @param format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
 * @param ... 格式化字符串参数列表
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_printf(int16_t x, int16_t y, uint8_t font_size, char *format, ...)
{
    char string[256];
    va_list arg;                               // 定义可变参数列表数据类型的变量arg
    va_start(arg, format);                     // 从format开始，接收参数列表到arg变量
    vsprintf(string, format, arg);             // 使用vsprintf打印格式化字符串和参数列表到字符数组中
    va_end(arg);                               // 结束变量arg
    oled_show_string(x, y, string, font_size); // oled显示字符数组(字符串)
}

/**
 * @brief OLED在指定位置画一个点
 *
 * @param x 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_draw_point(int16_t x, int16_t y)
{
    if (x >= 0 && x <= 127 && y >= 0 && y <= 63)
    {
        // 将现存数组指定位置的一个bit数据置1
        oled_display_buf[y / 8][x] |= 0x01 << (y % 8);
    }
}

/**
 * @brief OLED获取指定位置点的值
 * @param x 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @return uint8_t 指定位置点是否处于点亮状态，1：点亮，0：熄灭
 */
uint8_t oled_get_point(int16_t x, int16_t y)
{
    if (x >= 0 && x <= 127 && y >= 0 && y <= 63) // 超出屏幕区域不读取
    {
        // 判断位置数据
        if (oled_display_buf[y / 8][x] & 0x01 << (y % 8))
        {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief OLED画线
 * @param x0 指定一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y0 指定一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param x1 指定另一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y1 指定另一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t x, y, dx, dy, d, incr_e, incr_ne, temp;
    int8_t yflag = 0, xyflag = 0;

    if (y0 == y1) // 横线单独处理
    {
        // 0号点x坐标大于1号点x坐标，则交换两点x坐标
        if (x0 > x1)
        {
            temp = x0;
            x0 = x1;
            x1 = temp;
        }
        // 遍历x坐标
        for (x = x0; x <= x1; x++)
        {
            oled_draw_point(x, y0); // 依次画点
        }
    }
    else if (x0 == x1) // 竖线单独处理
    {
        // 0号点y坐标大于1号点y坐标，则交换两点y坐标
        if (y0 > y1)
        {
            temp = y0;
            y0 = y1;
            y1 = temp;
        }
        // 遍历y坐标
        for (y = y0; y <= y1; y++)
        {
            oled_draw_point(x0, y);
        }
    }
    else // 斜线
    {
        // 使用Bresenham算法画直线，可以避免好事的浮点运算，效率高
        if (x0 > x1) // 0号点x坐标大于1号点x坐标
        {
            // 交换两个点的坐标 交换后不影响画线，但是画线方向由第一、二、三、四象限变为第一、四象限
            temp = x0;
            x0 = x1;
            x1 = temp;
            temp = y0;
            y0 = y1;
            y1 = temp;
        }
        if (y0 > y1) // 0号点y坐标大于1号点y坐标
        {
            // 将y坐标取负 取负后影响画线，单画线方向由第一、四象限变为第一象限
            y0 = -y0;
            y1 = -y1;

            // 置标志位yflag，记住当前变换，在后续实际画线时，再将坐标换回来
            yflag = 1;
        }

        if (y1 - y0 > x1 - x0) // 画线斜率大于1
        {
            // 将x坐标与y坐标互换 互换后影响画线，但是画线方向由第一象限0~90度范围变为第一象限0~45度范围
            temp = x0;
            x0 = y0;
            y0 = temp;
            temp = x1;
            x1 = y1;
            y1 = temp;

            // 置标志位xyflag，记住当前变换，在后续实际画线时，再将坐标换回来
            xyflag = 1;
        }

        // 画线，画线方向必须为第一象限0~45度范围
        dx = x1 - x0;
        dy = y1 - y0;
        incr_e = 2 * dy;
        incr_ne = 2 * (dy - dx);
        d = 2 * dy - dx;
        x = x0;
        y = y0;

        // 画起始点，同时判断标志位，将坐标换回来
        if (yflag && xyflag)
            oled_draw_point(y, -x);
        else if (yflag)
            oled_draw_point(x, -y);
        else if (xyflag)
            oled_draw_point(y, x);
        else
            oled_draw_point(x, y);

        while (x < x1) // 遍历X轴的每一个点
        {
            x++;
            if (d < 0) // 下一个点在当前点东方
                d += incr_e;
            else // 下一个点在当前点东北方
            {
                y++;
                d += incr_ne;
            }
            // 画每一个点，同时判断标识位，将坐标换回来
            if (yflag && xyflag)
                oled_draw_point(y, -x);
            else if (yflag)
                oled_draw_point(x, -y);
            else if (xyflag)
                oled_draw_point(y, x);
            else
                oled_draw_point(x, y);
        }
    }
}

/**
 * @brief OLED矩形
 *
 * @param x 指定矩形左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定矩形左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param width  指定矩形的宽度，范围：0~128
 * @param height 指定矩形的高度，范围：0~64
 * @param is_filled 指定矩形是否填充
 *                  范围：OLED_UNFILLED		不填充
 *                  OLED_FILLED			填充
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_draw_rectangle(int16_t x, int16_t y, uint8_t width, uint8_t height, uint8_t is_filled)
{
    int i, j;
    if (!is_filled) // 指定矩形不填充
    {
        // 遍历上下x坐标，画矩形上下两条线
        for (i = x; i < x + width; i++)
        {
            oled_draw_point(i, y);
            oled_draw_point(i, y + height - 1);
        }
        // 遍历左右y坐标，画矩形左右两条线
        for (i = y; i < y + height; i++)
        {
            oled_draw_point(x, i);
            oled_draw_point(x + width - 1, i);
        }
    }
    else // 指定矩形填充
    {
        // 遍历x坐标
        for (i = x; i < x + width; i++)
        {
            // 遍历y坐标
            for (j = y; j < y + height; j++)
            {
                // 在指定区域画点，填充矩形
                oled_draw_point(i, j);
            }
        }
    }
}

/**
 * @brief OLED三角形
 * @param x0 指定第一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y0 指定第一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param x1 指定第二个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y1 指定第二个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param x2 指定第三个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y2 指定第三个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param is_filled 指定三角形是否填充
 *                  范围：OLED_UNFILLED		不填充
 *                  OLED_FILLED			填充
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_draw_triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t is_filled)
{
    int16_t minx = x0, miny = y0, maxx = x0, maxy = y0;
    int16_t vx[] = {x0, x1, x2};
    int16_t vy[] = {y0, y1, y2};
    int i, j;

    if (!is_filled) // 指定三角形不填充
    {
        // 调用画线函数，将三个点直接用直线连接
        oled_draw_line(x0, y0, x1, y1);
        oled_draw_line(x0, y0, x2, y2);
        oled_draw_line(x1, y1, x2, y2);
    }
    else // 指定三角形填充
    {
        // 找到三个点最小的x、y坐标
        if (x1 < minx)
            minx = x1;
        if (x2 < minx)
            minx = x2;
        if (y1 < miny)
            miny = y1;
        if (y2 < miny)
            miny = y2;

        // 找到三个点最大的x、y坐标
        if (x1 > maxx)
            maxx = x1;
        if (x2 > maxx)
            maxx = x2;
        if (y1 > maxy)
            maxy = y1;
        if (y2 > maxy)
            maxy = y2;

        // 最小最大坐标之前的举行为可能需要填充的区域
        // 遍历此区域中所有的点
        // 遍历x坐标
        for (i = minx; i <= maxx; i++)
        {
            // 遍历y坐标
            for (j = miny; j <= maxy; j++)
            {
                // 调用oled_pnpoly,判断指定点是否在指定三角形中
                // 如果在，则画点，若不在，则不处理
                if (oled_pnpoly(3, vx, vy, i, j))
                    oled_draw_point(i, j);
            }
        }
    }
}

/**
 * @brief OLED画圆
 * @param x 指定圆的圆心横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定圆的圆心纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param radius 指定圆的半径，范围：0~255
 * @param is_filled 指定圆是否填充
 *           范围：OLED_UNFILLED		不填充
 *                 OLED_FILLED			填充
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_dram_circle(int16_t x, int16_t y, uint8_t radius, uint8_t is_filled)
{
    int16_t x1, y1, d, j;
    int i;
    d = 1 - radius;
    x1 = 0;
    y1 = radius;

    // 画每个八分之一圆弧的起始点
    oled_draw_point(x + x1, y + y1);
    oled_draw_point(x - x1, y - y1);
    oled_draw_point(x + y1, y + x1);
    oled_draw_point(x - y1, y - x1);

    if (is_filled) // 指定圆填充
    {
        // 遍历起始点y坐标
        for (i = -y1; i < y1; i++)
        { // 指定区域内画点，填充部分圆
            oled_draw_point(x, y + i);
        }
    }

    while (x1 < y1) // 遍历x轴的每个点
    {
        x1++;
        if (d < 0) // 下一个点在当前点东方
        {
            d += 2 * x1 + 1;
        }
        else // 下一个点在当前点东南方
        {
            y1--;
            d += 2 * (x1 - y1) + 1;
        }

        // 画每个八分点圆弧的点
        oled_draw_point(x + x1, y + y1);
        oled_draw_point(x + y1, y + x1);
        oled_draw_point(x - x1, y - y1);
        oled_draw_point(x - y1, y - x1);
        oled_draw_point(x + x1, y - y1);
        oled_draw_point(x + y1, y - x1);
        oled_draw_point(x - x1, y + y1);
        oled_draw_point(x - y1, y + x1);

        if (is_filled) // 制定预案填充
        {
            // 遍历中间部分
            for (j = -y1; j < y1; j++)
            {
                oled_draw_point(x + x1, y + j);
                oled_draw_point(x - x1, y + j);
            }
            // 遍历两侧部分
            for (j = -x; j < x; j++)
            {
                oled_draw_point(x - y1, y + j);
                oled_draw_point(x + y1, y + j);
            }
        }
    }
}

/**
 * @brief OLED画椭圆
 * @param x 指定椭圆的圆心横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定椭圆的圆心纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param A 指定椭圆的横向半轴长度，范围：0~255
 * @param B 指定椭圆的纵向半轴长度，范围：0~255
 * @param is_filled IsFilled 指定椭圆是否填充
 *                  范围：OLED_UNFILLED		不填充
 *                  OLED_FILLED			填充
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_draw_ellipse(int16_t x, int16_t y, uint8_t A, uint8_t B, uint8_t is_filled)
{
    int16_t x1, y1, j;
    int16_t a = A, b = B;
    float d1, d2;

    x1 = 0;
    y1 = b;
    d1 = b * b + a * a * (-b + 0.5);

    if (is_filled) // 指定椭圆填充
    {
        // 遍历起始点y坐标
        for (j = -y1; j < y1; j++)
        {
            // 在指定区域画圆，填充部分椭圆
            oled_draw_point(x, y + j);
            oled_draw_point(x, y + j);
        }
    }

    // 画椭圆弧的起始点
    oled_draw_point(x + x1, y + y1);
    oled_draw_point(x - x1, y - y1);
    oled_draw_point(x - x1, y + y1);
    oled_draw_point(x + x1, y - y1);

    // 画椭圆中间部分
    while (b * b * (x1 + 1) < a * a * (y1 - 0.5))
    {
        if (d1 <= 0) // 下一个点在当前点东方
            d1 += b * b * (2 * x1 + 3);
        else // 下一个点在当前点东南方
        {
            d1 += b * b * (2 * x1 + 3) + a * a * (-2 * y1 + 2);
            y--;
        }
        x++;

        if (is_filled) // 指定椭圆填充
        {
            // 遍历中间部分
            for (j = -y; j < y1; j++)
            {
                // 在指定区域画点，填充部分椭圆
                oled_draw_point(x + x1, y + j);
                oled_draw_point(x - x1, y + j);
            }
        }
        // 画椭圆中间部分圆弧
        oled_draw_point(x + x1, y + y1);
        oled_draw_point(x - x1, y - y1);
        oled_draw_point(x - x1, y + y1);
        oled_draw_point(x + x1, y - y1);
    }
    // 画椭圆两侧部分
    d2 = b * b * (x1 + 0.5) * (x1 + 0.5) + a * a * (y1 - 1) * (y1 - 1) - a * a * b * b;

    while (y > 0)
    {
        if (d2 <= 0) // 下一个点在当前点东方
        {
            d2 += b * b * (2 * x1 + 2) + a * a * (-2 * y1 + 3);
            x1++;
        }
        else // 下一个点在当前点东南方
        {
            d2 += a * a * (-2 * y1 + 3);
        }
        y1--;

        if (is_filled) // 指定椭圆填充
        {
            // 遍历两侧部分
            for (j = -y1; j < y1; j++)
            {
                // 在指定区域画点，填充部分椭圆
                oled_draw_point(x + x1, y + j);
                oled_draw_point(x - x1, y + j);
            }
        }
        oled_draw_point(x + x1, y + y1);
        oled_draw_point(x - x1, y - y1);
        oled_draw_point(x - x1, y + y1);
        oled_draw_point(x + x1, y - y1);
    }
}

/**
 * @brief OLED画圆弧
 * @param x 指定圆弧的圆心横坐标，范围：-32768~32767，屏幕区域：0~127
 * @param y 指定圆弧的圆心纵坐标，范围：-32768~32767，屏幕区域：0~63
 * @param radius 指定圆弧的半径，范围：0~255
 * @param start_angle StartAngle 指定圆弧的起始角度，范围：-180~180
 *                    水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
 * @param end_angle 指定圆弧的终止角度，范围：-180~180
 *                  水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
 * @param is_filled 指定圆弧是否填充，填充后为扇形
 *                  范围：OLED_UNFILLED		不填充
 *                  OLED_FILLED			填充
 * @details 调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
 */
void oled_draw_arc(int16_t x, int16_t y, uint8_t radius, int16_t start_angle, int16_t end_angle, uint8_t is_filled)
{
    int16_t x1, y1, d, j;

    d = 1 - radius;
    x1 = 0;
    y1 = radius;

    if (oled_is_in_angle(x1, y1, start_angle, end_angle))
        oled_draw_point(x + x1, y + y1);
    if (oled_is_in_angle(-x1, -y1, start_angle, end_angle))
        oled_draw_point(x - x1, y - y1);
    if (oled_is_in_angle(y1, x1, start_angle, end_angle))
        oled_draw_point(x + y1, y + x1);
    if (oled_is_in_angle(-y1, -x1, start_angle, end_angle))
        oled_draw_point(x - y1, y + x1);

    if (is_filled) // 指定圆弧填充
    {
        // 遍历起始点y坐标
        for (j = -y1; j < y1; j++)
        {
            // 在填充圆的每个点时，判断指定点是否在指定角度内，在则画点，不在则不处理
            if (oled_is_in_angle(0, j, start_angle, end_angle))
                oled_draw_point(x, y + j);
        }
    }
    while (x1 < y1) // 遍历x轴的每个点
    {
        x1++;
        if (d < 0) // 下一个点在东方
        {
            d += 2 * x1 + 1;
        }
        else // 下一个点在东南方
        {
            y1--;
            d += 2 * (x1 - y1) + 1;
        }
        // 在画每个点时，判断指定点是否在指定角度内，在则画，不在则不处理
        if (oled_is_in_angle(x1, y1, start_angle, end_angle))
            oled_draw_point(x + x1, y + y1);
        if (oled_is_in_angle(y1, x1, start_angle, end_angle))
            oled_draw_point(x + y1, y + x1);
        if (oled_is_in_angle(-x1, -y1, start_angle, end_angle))
            oled_draw_point(x - x1, y - y1);
        if (oled_is_in_angle(-y1, -x1, start_angle, end_angle))
            oled_draw_point(x - y1, y - x1);
        if (oled_is_in_angle(x1, -y1, start_angle, end_angle))
            oled_draw_point(x + x1, y - y1);
        if (oled_is_in_angle(y1, -x1, start_angle, end_angle))
            oled_draw_point(x + y1, y - x1);
        if (oled_is_in_angle(-x1, y1, start_angle, end_angle))
            oled_draw_point(x - x1, y + y1);
        if (oled_is_in_angle(-y1, x1, start_angle, end_angle))
            oled_draw_point(x - y1, y + x1);

        if (is_filled) // 指定圆弧填充
        {
            // 遍历中间部分
            for (j = -y1; j < y1; j++)
            {
                // 在填充圆的每个点时，判断指定点是否在指定角度内，在则画点，不在则不处理
                if (oled_is_in_angle(x1, j, start_angle, end_angle))
                    oled_draw_point(x + x1, y + j);
                if (oled_is_in_angle(-x1, j, start_angle, end_angle))
                    oled_draw_point(x - x1, y + j);
            }

            // 遍历两侧部分
            for (j = -x1; j < x1; j++)
            {
                // 在填充圆的每个点时，判断指定点是否在指定角度内，在则画点，不在则不处理
                if (oled_is_in_angle(-y1, j, start_angle, end_angle))
                    oled_draw_point(x - y1, y + j);
                if (oled_is_in_angle(y1, j, start_angle, end_angle))
                    oled_draw_point(x + y1, y + j);
            }
        }
    }
}
