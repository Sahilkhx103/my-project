#include "timer.h"

void TIM6_Init_10kHz_TRGO(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

    TIM6->PSC = 47U;

    TIM6->ARR = 99U;

    TIM6->CR2 &= ~TIM_CR2_MMS;
    TIM6->CR2 |= (2U << TIM_CR2_MMS_Pos);

    TIM6->EGR |= TIM_EGR_UG;

    TIM6->CR1 |= TIM_CR1_CEN;
}