#ifndef ADC_H
#define ADC_H

#include "stm32f303xc.h"
#include <stdint.h>

#define ADC_BUF_SIZE 512U
#define ADC_BUF_MASK (ADC_BUF_SIZE - 1U)

void ADC1_Init_TIM6_Triggered(void);
uint8_t ADC1_ReadSample(uint16_t *sample_out);
uint32_t ADC1_GetOverflowCount(void);
uint32_t ADC1_GetLastSecondCount(void);

#endif 