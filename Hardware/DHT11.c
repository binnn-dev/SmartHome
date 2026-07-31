#include "DHT11.h"
#include "Delay.h"

/* DHT11 单总线空闲时需要保持高电平。
   发送起始信号时切换成推挽输出，由 MCU 主动拉低总线。
   当前数据脚已从 PA3 改为 PA4。 */
static void DHT11_Mode_Out(void) {
    GPIOA->CRL &= 0xFFF0FFFF;
    GPIOA->CRL |= 0x00030000;
}

/* 读取 DHT11 数据时切回输入模式，并通过 ODR 保持上拉 */
static void DHT11_Mode_In(void) {
    GPIOA->CRL &= 0xFFF0FFFF;
    GPIOA->CRL |= 0x00080000;
    GPIOA->ODR |= GPIO_Pin_4;
}

void DHT11_Init(void) {
    DHT11_Mode_In();
}

uint8_t DHT11_Read(uint8_t *temp, uint8_t *humi) {
    uint8_t i, j;
    uint8_t buf[5] = {0};

    /* 起始时序：主机先拉低至少 18ms，再释放总线，通知 DHT11 准备发数据 */
    DHT11_Mode_Out();
    GPIOA->BRR = GPIO_Pin_4;
    Delay_ms(20);
    GPIOA->BSRR = GPIO_Pin_4;
    Delay_us(30);

    DHT11_Mode_In();
    if (!(GPIOA->IDR & GPIO_Pin_4)) {
        /* 跳过 DHT11 的响应低电平和高电平 */
        while (!(GPIOA->IDR & GPIO_Pin_4));
        while (GPIOA->IDR & GPIO_Pin_4);

        /* 依次读取 40bit：
           湿度整数、湿度小数、温度整数、温度小数、校验和 */
        for (i = 0; i < 5; i++) {
            for (j = 0; j < 8; j++) {
                while (!(GPIOA->IDR & GPIO_Pin_4));
                /* DHT11 用高电平持续时间区分 0/1，这里在 40us 处采样 */
                Delay_us(40);
                if (GPIOA->IDR & GPIO_Pin_4) {
                    buf[i] |= (1 << (7 - j));
                    while (GPIOA->IDR & GPIO_Pin_4);
                }
            }
        }

        /* 校验通过后只取整数部分，当前项目和安卓端都按整数温湿度处理 */
        if ((uint8_t)(buf[0] + buf[1] + buf[2] + buf[3]) == buf[4]) {
            *humi = buf[0];
            *temp = buf[2];
            return 1;
        }
    }

    return 0;
}
