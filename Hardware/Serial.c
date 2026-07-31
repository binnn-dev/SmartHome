#include "Serial.h"
#include "Delay.h"

void Serial_Init(uint32_t baud_rate) {
    GPIO_InitTypeDef gpio_init;
    USART_InitTypeDef usart_init;

    /* 改用硬件 USART2：
       PA2 -> ESP8266 RX
       PA3 <- ESP8266 TX */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    gpio_init.GPIO_Pin = GPIO_Pin_2;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpio_init);

    gpio_init.GPIO_Pin = GPIO_Pin_3;
    gpio_init.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &gpio_init);

    usart_init.USART_BaudRate = baud_rate;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &usart_init);
    USART_Cmd(USART2, ENABLE);
    Delay_ms(20);
}

void Serial_SendString(const char *string) {
    while (*string) {
        USART_SendData(USART2, (uint16_t)*string++);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}

uint8_t Serial_ReadByte(uint8_t *byte, uint32_t timeout_ms) {
    uint32_t elapsed_ms = 0;

    while (elapsed_ms < timeout_ms) {
        if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET) {
            *byte = (uint8_t)USART_ReceiveData(USART2);
            return 1;
        }
        Delay_ms(1);
        elapsed_ms++;
    }

    return 0;
}

uint16_t Serial_ReadResponse(char *buffer, uint16_t buffer_size, uint32_t timeout_ms) {
    uint16_t count = 0;
    uint8_t byte = 0;
    uint32_t idle_ms = 0;

    if (buffer_size == 0) {
        return 0;
    }

    while (idle_ms < timeout_ms && count < (uint16_t)(buffer_size - 1)) {
        if (Serial_ReadByte(&byte, 1)) {
            buffer[count++] = (char)byte;
            idle_ms = 0;
        } else {
            idle_ms++;
        }
    }

    buffer[count] = '\0';
    return count;
}
