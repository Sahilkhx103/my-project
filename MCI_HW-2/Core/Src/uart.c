#include "uart.h"

void USART1_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    GPIOC->MODER &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5);
    GPIOC->MODER |= (2U << GPIO_MODER_MODER4_Pos) | (2U << GPIO_MODER_MODER5_Pos);

    GPIOC->OSPEEDR |= (3U << GPIO_OSPEEDER_OSPEEDR4_Pos);

    GPIOC->AFR[0] &= ~((0xFU << 16U) | (0xFU << 20U));
    GPIOC->AFR[0] |= (7U << 16U) | (7U << 20U);

    USART1->CR1 &= ~USART_CR1_UE;

    USART1->CR1 &= ~USART_CR1_M;

    USART1->CR1 &= ~(USART_CR1_PCE | USART_CR1_OVER8);

    USART1->CR2 &= ~USART_CR2_STOP;

    USART1->BRR = 208U;

    USART1->CR1 |= (USART_CR1_TE | USART_CR1_UE);
}

void USART1_SendByte(uint8_t byte)
{
    while (!(USART1->ISR & USART_ISR_TXE)) {
    }
    USART1->TDR = byte;
}

void USART1_SendSample12Bit(uint16_t sample)
{
    uint8_t high_byte = (uint8_t)(0x80U | ((sample >> 6U) & 0x3FU));
    uint8_t low_byte  = (uint8_t)(0x00U | (sample & 0x3FU));

    USART1_SendByte(high_byte);
    USART1_SendByte(low_byte);
}