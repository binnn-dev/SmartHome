#ifndef __BSP_CONFIG_H
#define __BSP_CONFIG_H

#include "stm32f10x.h"

#define LED_RED(x)      (x ? (GPIOB->BRR = GPIO_Pin_13) : (GPIOB->BSRR = GPIO_Pin_13))
#define LED_GREEN(x)    (x ? (GPIOB->BRR = GPIO_Pin_12) : (GPIOB->BSRR = GPIO_Pin_12))
#define BEEP(x)         (x ? (GPIOA->BSRR = GPIO_Pin_12) : (GPIOA->BRR = GPIO_Pin_12))

#define ESP_EN(x)       (x ? (GPIOB->BSRR = GPIO_Pin_4) : (GPIOB->BRR = GPIO_Pin_4))
#define ESP_RST(x)      (x ? (GPIOB->BSRR = GPIO_Pin_5) : (GPIOB->BRR = GPIO_Pin_5))
#define ESP_IO0(x)      (x ? (GPIOB->BSRR = GPIO_Pin_6) : (GPIOB->BRR = GPIO_Pin_6))
#define ESP_IO2(x)      (x ? (GPIOB->BSRR = GPIO_Pin_7) : (GPIOB->BRR = GPIO_Pin_7))

#define GET_DOOR        (GPIOA->IDR & GPIO_Pin_0)
#define GET_PIR         (GPIOA->IDR & GPIO_Pin_5)
#define GET_SMOKE_DO    (GPIOA->IDR & GPIO_Pin_6)
#define GET_GAS_DO      (GPIOA->IDR & GPIO_Pin_10)

void Bsp_Init(void);
uint16_t Get_ADC_Value(uint8_t adc_channel);

#endif
