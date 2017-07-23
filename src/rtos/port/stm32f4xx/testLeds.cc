#include "stm32f4xx.h"
#include "core_cm4.h"

#include <rtos/base/testLeds.h>
#include <rtos/base/assert.h>


namespace Rtos {
namespace Base {

const int TestLeds::amount = 4;

void TestLeds::doSet(int n, bool on) {
  constexpr static GPIO_TypeDef *port = GPIOD;
  constexpr static uint16_t pinMasks[amount] = { GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15 };

  RTOS_ASSERT(n >= 0 && n < amount);

  const uint16_t pinMask = pinMasks[n];

  if(on)
    GPIO_SetBits(port, pinMask);
  else
    GPIO_ResetBits(port, pinMask);
}


}} // Base, Rtos
