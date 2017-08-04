#include "stm32f4xx.h"
#include "core_cm4.h"

#include <stdem/testing/leds.h>
#include <stdem/testing/logger.h>
#include <stdem/testing/testCase.h>

#include <rtos/kernel/scheduler.h>

#include <functional>



void initUart();
void leds_Init();

void taskFunc(void*);


uint32_t thread0Stack[128] __attribute__((aligned (8)));
uint32_t thread1Stack[128] __attribute__((aligned (8)));


StdEm::Testing::Leds leds;
StdEm::Testing::Logger logger;


namespace Rtos {
  namespace Kernel {
    extern Scheduler scheduler;
}}

using StdEm::Testing::TestCase;

class TestRunner: public StdEm::Testing::TestRunner
{
protected:
  virtual void hardwareReset() override
    { asm("svc #2"); }
};

struct ThreadArg {
  int       m_num;
  TestCase* m_testCase;
  int64_t   m_counter;

  ThreadArg(int num, TestCase* testCase)
  {
    m_num       = num;
    m_testCase  = testCase;
    m_counter   = 0;
  }
};

ThreadArg* threadArgs[2];

class Test1: public TestCase {
public:
  Test1(): TestCase("Test1") {
    addTest("test1", [this] () {
      threadArgs[0] = new ThreadArg(0, this);
      threadArgs[1] = new ThreadArg(1, this);

      Rtos::Kernel::scheduler.addThread(new Rtos::Kernel::Tcb(taskFunc, threadArgs[0], thread0Stack, sizeof(thread0Stack), 10));
      Rtos::Kernel::scheduler.addThread(new Rtos::Kernel::Tcb(taskFunc, threadArgs[1], thread1Stack, sizeof(thread1Stack), 10));
      Rtos::Kernel::scheduler.start();

      STDEM_ASSERT(false /* never should be here */;)
    });
  }
};


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

  Test1 testCase1;
  TestRunner testRunner;
  testRunner << testCase1;
  testRunner.run(logger);

  for(;;) {}
}

void finishTest()
{
  asm("svc #3");

  logger << "counter0: " << threadArgs[0]->m_counter << "\n";
  logger << "counter1: " << threadArgs[1]->m_counter << "\n";

  int64_t counterDiff = threadArgs[0]->m_counter - threadArgs[1]->m_counter;
  logger << "counterDiff: " << counterDiff << "\n";

  const int64_t maxCounterDiff = threadArgs[0]->m_counter / 10000; // 0.01%
  threadArgs[0]->m_testCase->finishTest(counterDiff < maxCounterDiff);
}

void taskFunc(void* _arg)
{
  // do not care to free memory from ThreadArg
  ThreadArg* arg = (ThreadArg*)_arg;

  uint32_t prevTick = getSysTick();
  const int threadNum = arg->m_num;
  uint32_t cnt = (threadNum == 0) ? 0 : 500;

  const int ledNum = (threadNum == 0) ? 0 : 2;

  while(true)
  {
    ++arg->m_counter;

    if(getSysTick() != prevTick)
    {
      prevTick = getSysTick();

      if(((cnt++) % 1000) == 0)
      {
        leds.toggle(ledNum);
        logger << "Thread #" << threadNum << " cnt= " << cnt << ", led: " << ledNum << "\n";
      }

      if(threadNum == 0 && cnt > 9999)
        finishTest();
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
