#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"


int main(void) {
  HAL_Init();
  SystemInit();

  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);

  BSP_LED_Toggle(LED3);

  volatile int i;
  i = 1;

  SysTick_Config (SystemCoreClock / 1000);

  volatile int t;
  volatile int c;
  for(;;)
  {
    t = SysTick->VAL;
    c = SysTick->CTRL;
  }
}

int tickCounter = 0;

extern "C" void SysTick_Handler() {
  tickCounter++;
  if((tickCounter % 500) == 0)
    BSP_LED_Toggle(LED4);
}
