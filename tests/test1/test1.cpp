#include "stm32f4xx.h"
#include <system_stm32f4xx.h>


int main(void) {
  SystemInit();

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

extern "C" void SysTick_Handler() {
  volatile int t;
  t = 0;
}
