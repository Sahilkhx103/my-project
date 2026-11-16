#include "adc.h"

static volatile uint16_t rx_buf[ADC_BUF_SIZE];
static volatile uint16_t head = 0U;
static volatile uint16_t tail = 0U;
static volatile uint32_t overflow_count = 0U;
static volatile uint32_t sec_counter = 0U;
static volatile uint32_t last_sec_count = 0U;

void ADC1_Init_TIM6_Triggered(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_ADC12EN;

    GPIOA->MODER |= (3U << GPIO_MODER_MODER1_Pos);

    GPIOA->PUPDR &= ~(3U << GPIO_PUPDR_PUPDR1_Pos);

    ADC12_COMMON->CCR &= ~ADC12_CCR_CKMODE;
    ADC12_COMMON->CCR |= ADC12_CCR_CKMODE_1;

    ADC1->CR &= ~ADC_CR_ADVREGEN;
    ADC1->CR |= ADC_CR_ADVREGEN_0;

    for (volatile uint32_t i = 0; i < 800U; i++) {
        __NOP();
    }

    ADC1->CR &= ~ADC_CR_ADCALDIF;

    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL) {
    }

    ADC1->CFGR &= ~ADC_CFGR_RES;

    ADC1->CFGR &= ~ADC_CFGR_ALIGN;

    ADC1->CFGR &= ~ADC_CFGR_CONT;

    ADC1->CFGR &= ~ADC_CFGR_EXTSEL;
    ADC1->CFGR |= (13U << ADC_CFGR_EXTSEL_Pos);

    ADC1->CFGR &= ~ADC_CFGR_EXTEN;
    ADC1->CFGR |= ADC_CFGR_EXTEN_0;

    ADC1->SQR1 = (2U << ADC_SQR1_SQ1_Pos);

    ADC1->SMPR1 &= ~ADC_SMPR1_SMP2;
    ADC1->SMPR1 |= (2U << ADC_SMPR1_SMP2_Pos);

    ADC1->IER |= ADC_IER_EOCIE;

    NVIC_SetPriority(ADC1_2_IRQn, 1U);
    NVIC_EnableIRQ(ADC1_2_IRQn);

    ADC1->ISR |= ADC_ISR_ADRDY;

    ADC1->CR |= ADC_CR_ADEN;
    while (!(ADC1->ISR & ADC_ISR_ADRDY)) {
    }

    ADC1->CR |= ADC_CR_ADSTART;
}

void ADC1_2_IRQHandler(void)
{
    if (ADC1->ISR & ADC_ISR_EOC) {
        uint16_t sample = (uint16_t)(ADC1->DR & 0x0FFFU);

        uint16_t next_head = (head + 1U) & ADC_BUF_MASK;
        if (next_head != tail) {
            rx_buf[head] = sample;
            head = next_head;
        } else {
            overflow_count++;
        }

        sec_counter++;
        if (sec_counter >= 10000U) {
            last_sec_count = sec_counter;
            sec_counter = 0U;
            GPIOE->ODR ^= GPIO_ODR_9;
        }
    }
}

uint8_t ADC1_ReadSample(uint16_t *sample_out)
{
    if (head == tail) {
        return 0U;
    }
    *sample_out = rx_buf[tail];
    tail = (tail + 1U) & ADC_BUF_MASK;
    return 1U;
}

uint32_t ADC1_GetOverflowCount(void)
{
    return overflow_count;
}

uint32_t ADC1_GetLastSecondCount(void)
{
    return last_sec_count;
}