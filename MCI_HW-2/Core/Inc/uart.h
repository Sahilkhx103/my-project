#ifndef UART_H
#define UART_H

#include "stm32f303xc.h"
#include <stdint.h>

void USART1_Init(void);
void USART1_SendByte(uint8_t byte);
void USART1_SendSample12Bit(uint16_t sample);

#endif /* UART_H */