#include "stm32f303xc.h"
#include "timer.h"
#include "adc.h"
#include "uart.h"

static void SystemClock_Config_48MHz(void)
{
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY)) {
    }

    FLASH->ACR |= FLASH_ACR_PRFTBE;
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_0;

    RCC->CFGR &= ~RCC_CFGR_HPRE;

    RCC->CFGR &= ~RCC_CFGR_PPRE1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    RCC->CFGR &= ~RCC_CFGR_PPRE2;

    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMUL);
    RCC->CFGR |= RCC_CFGR_PLLMUL12;

    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY)) {
    }

    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
    }
}

static void LED_Init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOEEN;

    GPIOE->MODER &= ~GPIO_MODER_MODER9;
    GPIOE->MODER |= (1U << GPIO_MODER_MODER9_Pos);
}

int main(void)
{
    uint16_t sample = 0U;

    SystemClock_Config_48MHz();
    LED_Init();
    USART1_Init();
    ADC1_Init_TIM6_Triggered();
    TIM6_Init_10kHz_TRGO();

    while (1) {
        if (ADC1_ReadSample(&sample)) {
            USART1_SendSample12Bit(sample);
        }
    }
}