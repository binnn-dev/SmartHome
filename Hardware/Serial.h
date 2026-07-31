#ifndef __SERIAL_H
#define __SERIAL_H

#include "stm32f10x.h"

void Serial_Init(uint32_t baud_rate);
void Serial_SendString(const char *string);
uint8_t Serial_ReadByte(uint8_t *byte, uint32_t timeout_ms);
uint16_t Serial_ReadResponse(char *buffer, uint16_t buffer_size, uint32_t timeout_ms);

#endif
