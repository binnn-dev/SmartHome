#include "Bsp_Config.h"

void Bsp_Init(void) {
    GPIO_InitTypeDef gpio_init;
    ADC_InitTypeDef adc_init;

    /* 打开主控中本项目会用到的时钟：
       GPIOA/GPIOB 用于传感器、LED、蜂鸣器、ESP、OLED
       AFIO 用于复用功能
       ADC1 用于烟雾 / 燃气模拟采样 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_AFIO |
                           RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* PB12/PB13: 绿灯 / 红灯 */
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_Init(GPIOB, &gpio_init);

    /* PA12: 蜂鸣器驱动 */
    gpio_init.GPIO_Pin = GPIO_Pin_12;
    GPIO_Init(GPIOA, &gpio_init);

    /* PB4~PB7: ESP8266 EN / RST / IO0 / IO2 控制脚 */
    gpio_init.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOB, &gpio_init);

    /* 数字输入：
       PA0  门磁
       PA5  红外
       PA6  烟雾数字输出
       PA10 燃气数字输出 */
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    gpio_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_10;
    GPIO_Init(GPIOA, &gpio_init);

    /* 模拟输入：
       PA7 -> 烟雾 ADC
       PB1 -> 燃气 ADC(ADC_Channel_9) */
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    gpio_init.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_1;
    GPIO_Init(GPIOB, &gpio_init);

    /* 上电默认关闭灯和蜂鸣器，并把 ESP8266 拉到正常启动状态 */
    LED_RED(0);
    LED_GREEN(0);
    BEEP(0);
    ESP_EN(1);
    ESP_RST(1);
    ESP_IO0(1);
    ESP_IO2(1);

    /* ADC1 配置成单次单通道采样，主循环中按需读取烟雾 / 燃气 */
    ADC_DeInit(ADC1);
    adc_init.ADC_Mode = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode = DISABLE;
    adc_init.ADC_ContinuousConvMode = DISABLE;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc_init);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET);
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET);
}

uint16_t Get_ADC_Value(uint8_t adc_channel) {
    /* 每次调用都临时切换到目标通道并做一次软件触发采样 */
    ADC_RegularChannelConfig(ADC1, adc_channel, 1, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC1);
}
