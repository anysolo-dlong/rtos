#include "stm32f4xx.h"
#include "core_cm4.h"

#include <stdem/testing/leds.h>
#include <stdem/testing/logger.h>
#include <stdem/testing/testCase.h>

#include <rtos/kernel/scheduler.h>

#include <functional>



void initUart();
void leds_Init();

void task0(void);
void task1(void);


uint32_t thread0Stack[128] __attribute__((aligned (8)));
uint32_t thread1Stack[128] __attribute__((aligned (8)));


StdEm::Testing::Leds leds;
StdEm::Testing::Logger logger;

#if 0
class Test1: public StdEm::Testing::TestCase
{
  int m_someData;

public:
  Test1(): TestCase("Test1")
  {
    m_someData = 100;

    addTest("test1", [this] ()
      {
        logger() << "test1 body " << m_someData << "\n";
        for(volatile long i = 0; i < 3000000; i++) ;
      }
    );

    addTest("test2", [this] ()
      {
        logger() << "test2 body " << m_someData << "\n";
        for(volatile long i = 0; i < 3000000; i++) ;
      }
    );

    addTest("test3", [this] ()
      {
        logger() << "test3 body " << m_someData << "\n";
        for(volatile long i = 0; i < 3000000; i++) ;
      }
    );
  }

  virtual void beforeTest()
  {
    ++m_someData;
  }
};

int testExt = 10;
#endif

namespace Rtos {
  namespace Kernel {
    extern Scheduler scheduler;
}}

// TODO lock?
inline Rtos::Kernel::Tick getSysTick() {
  return Rtos::Kernel::scheduler.tickCounter();
}

int main(void) {
  for(volatile long i = 0; i < 3000000; i++) ;

  SystemInit();

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  leds_Init();

  initUart();
  logger << "test\n";

  Rtos::Kernel::scheduler.addThread(new Rtos::Kernel::Tcb(task0, 0, thread0Stack, sizeof(thread0Stack), 10));
  Rtos::Kernel::scheduler.addThread(new Rtos::Kernel::Tcb(task1, 0, thread1Stack, sizeof(thread1Stack), 10));
  Rtos::Kernel::scheduler.start();

//  logger << "test" << testExt << "\n";
  for(volatile long i = 0; i < 3000000; i++) ;

//  Test1 testCase1;
//  StdEm::Testing::TestRunner testRunner;
//  testRunner << testCase1;
//  NVIC_SystemReset();
//  testRunner.run(logger);

  for(;;) {}
}

void initUart()
{
  GPIO_InitTypeDef GPIO_InitStruct;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_USART2);

  USART_InitTypeDef USART_InitStruct;

  USART_InitStruct.USART_BaudRate = 115200;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  USART_Init(USART2, &USART_InitStruct);

  USART_Cmd(USART2, ENABLE);
}

extern "C" int _write(int file, char *ptr, int len)
{
  if(!ptr)
    return 0;

  for(int i = 0; i < len; i++) {
    while (!(USART2->SR & 0x00000040));
    USART_SendData(USART2, ptr[i]);
  }

  return len;
}


void task0(void)
{
  uint32_t prevTick = getSysTick();
  uint32_t cnt = 0;

  while(true)
  {
    if(getSysTick() != prevTick) {
      prevTick = getSysTick();

      if(((cnt++) % 1000) == 0) {
        leds.toggle(0);
        logger << "Thread1" << " cnt= " << cnt << "\n";
      }
    }
  }
}

void task1(void)
{
  uint32_t prevTick = getSysTick();
  uint32_t cnt = 500;

  while(true)
  {
    if(getSysTick() != prevTick) {
      prevTick = getSysTick();

      if(((cnt++) % 1000) == 0)
        leds.toggle(2);
    }
  }
}

void leds_Init() {
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Clock for GPIOD */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

  /* Set pins */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStruct);
}
