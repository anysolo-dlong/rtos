#include "stm32f4xx.h"
#include "core_cm4.h"

#include <stdem/testing/leds.h>
#include <stdem/testing/logger.h>
#include <stdem/testing/testCase.h>

#include <functional>


struct StackFrame1
{
  uint32_t
    r0, r1, r2, r3,
    r12, lr, pc, xpsr
  ;
};

struct StackFrameAddRegs {
  uint32_t
    r4, r5, r6, r7, r8, r9, r10, r11, r14
  ;

};

struct FullStackFrame {
  StackFrameAddRegs addRegs;
  StackFrame1       subFrame1;
};


void initUart();
void leds_Init();

void rtos_init();
extern "C" void svcHandler_C(void*);
extern "C" void* pendSvHandler_C(void* _stackFrame);

void task0(void);
void task1(void);

void rtos_startFirstTask(StackFrame1& initialFrame);

volatile int currentTask = 0;

volatile uint32_t thread0Stack[128] __attribute__((aligned (8)));
volatile uint32_t thread1Stack[128] __attribute__((aligned (8)));

volatile void* threadPsp[2];

volatile uint32_t tickCounter = 0;

StdEm::Testing::Leds leds;
StdEm::Testing::Logger logger;

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

int main(void) {
  for(volatile long i = 0; i < 3000000; i++) ;

  SystemInit();

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  leds_Init();

//  SysTick_Config (SystemCoreClock / 1000);

  initUart();
  logger << "test\n";
  logger << "test" << testExt << "\n";
  for(volatile long i = 0; i < 3000000; i++) ;

  Test1 testCase1;
  StdEm::Testing::TestRunner testRunner;
  testRunner << testCase1;
//  NVIC_SystemReset();
  testRunner.run(logger);

  for(;;) {}

  NVIC_SetPriority(PendSV_IRQn, 0xFF); // Set PendSV to lowest possible priority
  SCB->CCR |= SCB_CCR_STKALIGN_Msk;
  __set_PSP(uint32_t(thread0Stack) + sizeof(thread0Stack));

  __ISB();
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivileged state
  __ISB();
  asm("svc #1");

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
  uint32_t prevTick = tickCounter;
  uint32_t cnt = 0;

  while(true)
  {
    if(tickCounter != prevTick) {
      prevTick = tickCounter;

      if(((cnt++) % 1000) == 0) {
        leds.toggle(0);
        logger << "Thread1" << " cnt= " << cnt << "\n";
      }
    }
  }
}

void task1(void)
{
  uint32_t prevTick = tickCounter;
  uint32_t cnt = 500;

  while(true)
  {
    if(tickCounter != prevTick) {
      prevTick = tickCounter;

      if(((cnt++) % 1000) == 0)
        leds.toggle(2);
    }
  }
}

extern "C" void SVC_Handler()__attribute__ ((naked));

extern "C" void SVC_Handler()
{
  asm volatile
  (
      "tst lr, #4\n"
      "ite eq\n"
      "mrseq r0, msp\n"
      "mrsne r0, psp\n"
      "push {lr}\n"
      "bl svcHandler_C\n"
      "pop {lr}\n"
      "bx lr\n"
      ".align 4\n"
  );
}

extern "C" void svcHandler_C(void* _stackFrame)
{
  StackFrame1* frame = (StackFrame1*)_stackFrame;
  uint8_t svcArg = *((const uint8_t*)(int)frame->pc-2);

  if(svcArg == 1)
    rtos_startFirstTask(*frame);
}

void rtos_startFirstTask(StackFrame1& initialFrame)
{
  threadPsp[0] = (void*)thread0Stack + sizeof(thread0Stack) - sizeof(FullStackFrame);
  threadPsp[1] = (void*)thread1Stack + sizeof(thread0Stack) - sizeof(FullStackFrame);

  FullStackFrame* frame = (FullStackFrame*)threadPsp[0];
  frame->subFrame1 = initialFrame;
  frame->subFrame1.pc = uint32_t(task0);

  frame = (FullStackFrame*)threadPsp[1];
  frame->subFrame1 = initialFrame;
  frame->subFrame1.pc = uint32_t(task1);
}

extern "C" void SysTick_Handler()
{
  tickCounter++;
  if((tickCounter % 500) == 0)
    leds.toggle(3);

//  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set PendSV to pending
}

extern "C" void PendSV_Handler() __attribute__ ((naked));
extern "C" void PendSV_Handler()
{
  asm volatile
  (
      "push {lr}\n"
      "mrs r0, psp\n"
      "isb\n"
      "stmdb r0!, {r4-r11, r14}\n"
      "bl pendSvHandler_C\n"
      "ldmia r0!, {r4-r11, r14}\n"
      "msr psp, r0\n"
      "isb\n"
      "pop {lr}\n"
      "bx lr\n"
      ".align 4\n"
  );
}

extern "C" void* pendSvHandler_C(void* _stackFrame)
{
  threadPsp[currentTask] = _stackFrame;

  if(++currentTask == 2)
    currentTask = 0;

  return (void*)threadPsp[currentTask];
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
