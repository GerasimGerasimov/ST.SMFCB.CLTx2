#include "stm32f10x_it.h"
#include "stm32f10x.h"
#include "bastypes.h"
#include "uart1rs485.h"
#include "digits_display.h"

void TIM1_UP_TIM16_IRQHandler (void)
{
  TIM1_user_U1();
}

void TIM3_IRQHandler(void)
{
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
  outDigitsToSPI();  
}