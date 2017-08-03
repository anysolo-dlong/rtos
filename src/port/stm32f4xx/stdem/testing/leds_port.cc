#include "stm32f4xx.h"
#include "core_cm4.h"

#include <stdem/testing/leds.h>
#include <stdem/assert.h>


namespace StdEm {
namespace Testing {


void Leds::doSet(int n, bool on) {
  constexpr static GPIO_TypeDef *port = GPIOD;
  constexpr static uint16_t pinMasks[amountOfLeds] = { GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15 };

  STDEM_ASSERT(n >= 0 && n < amountOfLeds);

  const uint16_t pinMask = pinMasks[n];

  if(on)
    GPIO_SetBits(port, pinMask);
  else
    GPIO_ResetBits(port, pinMask);
}


}} // Testing, StdEm
