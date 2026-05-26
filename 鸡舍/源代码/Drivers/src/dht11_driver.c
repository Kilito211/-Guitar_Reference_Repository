#include "dht11_driver.h"

/**
 * @brief 端口变为输出
 */
static void dht11_io_out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_SWITCH_GPIO_PIN; // PA8
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_SWITCH_PORT, &GPIO_InitStructure);
}

/**
 * @brief 端口变为输入
 */
static void dht11_io_in(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_SWITCH_GPIO_PIN; // PA8
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_SWITCH_PORT, &GPIO_InitStructure);
}

/**
 * @brief DHT11复位
 */
static void dht11_reset(void)
{
    dht11_io_out();
    GPIO_ResetBits(DHT11_SWITCH_PORT, DHT11_SWITCH_GPIO_PIN);
    timer_delay_ms(20); // 等待拉低成功
    GPIO_SetBits(DHT11_SWITCH_PORT, DHT11_SWITCH_GPIO_PIN);
    timer_delay_ms(30); // 等待拉高成功
}

/**
 * @brief 等待DHT11回应
 *
 * @return uint8_t 成功返回0 失败返回1
 */
static uint8_t dht11_check(void)
{
    uint8_t retry = 0;
    // io切换为输入
    dht11_io_in();
    // 等待DHT11拉低总线
    while (GPIO_ReadInputDataBit(DHT11_SWITCH_PORT, DHT11_SWITCH_GPIO_PIN) && retry < 100)
    {
        retry++;
        timer_delay_us(1);
    }
    if (retry >= 100) // 超时返回失败
        return 1;

    // 等待DHT11释放总线
    while (!GPIO_ReadInputDataBit(DHT11_SWITCH_PORT, DHT11_SWITCH_GPIO_PIN) && retry < 100)
    {
        retry++;
        timer_delay_us(1);
    }
    if (retry >= 100)
        return 1; // 超时返回失败

    return 0; // 两次电平切换都检测到=应答成功
}

/**
 * @brief DHT11读取1位
 *
 * @return uint8_t 1/0
 */
static uint8_t dht11_read_bit(void)
{
    uint8_t retry = 0;
    // 等待低电平到来
    while (GPIO_ReadInputDataBit(DHT11_SWITCH_PORT, DHT11_SWITCH_GPIO_PIN) && retry < 100)
    {
        retry++;
        timer_delay_us(1);
    }
    retry = 0;

    // 等待低电平结束
    while (!GPIO_ReadInputDataBit(DHT11_SWITCH_PORT, DHT11_SWITCH_GPIO_PIN) && retry < 100)
    {
        retry++;
        timer_delay_us(1);
    }
    timer_delay_us(40);

    // 读取有效信号
    if (GPIO_ReadInputDataBit(DHT11_SWITCH_PORT, DHT11_SWITCH_GPIO_PIN))
        return 1;
    else
        return 0;
}

/**
 * @brief 从DHT11读取一个字节
 *
 * @return uint8_t 读取到的数据
 */
static uint8_t dht11_read_byte(void)
{
    uint8_t i, data;
    data = 0;
    for (i = 0; i < 8; i++)
    {
        data <<= 1;
        data |= dht11_read_bit();
    }

    return data;
}

/**
 * @brief 初始化dht11
 *
 * @return uint8_t 成功返回0 失败返回1
 */
uint8_t dht11_init(void)
{
    RCC_APB2PeriphClockCmd(DHT11_SWITCH_SYSCTL_PERIPH_CLK, ENABLE);
    dht11_reset();        // 复位dht11 发出起始信号
    return dht11_check(); // 等待应答信号
}

/**
 * @brief DHT11读取一次数据
 *
 * @param data 用于接收数据的变量地址
 * @return uint8_t 成功返回0 失败返回1
 */
uint8_t dht11_read_data(uint8_t *data)
{
    uint8_t buf[5];
    uint8_t i;

    dht11_reset(); // 端口复位发出起始信号

    if (!dht11_check()) // 等待应答信号
    {
        for (i = 0; i < 5; i++) // 读取5字节数据
        {
            buf[i] = dht11_read_byte();
        }
        if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *data = buf[0]; // 湿度
            data++;
            *data = buf[2]; // 温度
        }
    }
    else
        return 1;

    return 0;
}