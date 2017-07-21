#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4_discovery.h"
#include "core_cm4.h"


struct StackFrame1
{
  uint32_t
    r0,
    r1,
    r2,
    r3,
    r12,
    lr,
    pc,
    xpsr
  ;
};

void rtos_init();
extern "C" void svcHandler_C(void*);
extern "C" void pendSvHandler_C(void* _stackFrame);

void task0(void);
void task1(void);

void rtos_startFirstTask(StackFrame1& initialFrame);



StackFrame1 threadStackFrames[2];
int currentTask = 0;

void os_init()
{

}


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

  //  __set_CONTROL(0x3); // Switch to use Process Stack, unprivileged state
  //  __ISB();
    asm("svc #1");

  volatile int t;
  volatile int c;
  for(;;)
  {
    t = SysTick->VAL;
    c = SysTick->CTRL;
  }
}

int tickCounter = 0;

void task0(void)
{
  while(true)
  {
     if((tickCounter % 500) == 0)
       BSP_LED_Toggle(LED5);
  }
}

void task1(void)
{
  while(true)
  {
     if(((tickCounter + 200) % 500) == 0)
       BSP_LED_Toggle(LED6);
  }
}

extern "C" void SVC_Handler()__attribute__ ((naked));

extern "C" void SVC_Handler()
{
  asm
  (
      "tst lr, #4\n"
      "ite eq\n"
      "mrseq r0, msp\n"
      "mrsne r0, psp\n"
      "b svcHandler_C\n"
      ".align 4\n"
  );
}

extern "C" void svcHandler_C(void* _stackFrame)
{
  StackFrame1* frame = (StackFrame1*)_stackFrame;
  uint8_t svcArg = *((const uint8_t*)(int)frame->pc-2);

  if(svcArg == 1)
    rtos_startFirstTask(*frame);

  volatile int i;
  i = 1;
}

void rtos_startFirstTask(StackFrame1& initialFrame)
{
  threadStackFrames[0] = initialFrame;
  threadStackFrames[0].lr = 0;

  threadStackFrames[0].pc = (uint32_t)task0;
  threadStackFrames[1].pc = (uint32_t)task1;

  initialFrame.pc = threadStackFrames[0].pc;
}

extern "C" void SysTick_Handler()
{
  tickCounter++;
  if((tickCounter % 500) == 0)
    BSP_LED_Toggle(LED3);

  if(++currentTask == 2)
    currentTask = 0;

//  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set PendSV to pending
}

extern "C" void PendSV_Handler()
{
/*
  asm
  (
      "tst lr, #4\n"
      "ite eq\n"
      "mrseq r0, msp\n"
      "mrsne r0, psp\n"
      "b pendSvHandler_C\n"
      ".align 4\n"
  );
*/
}

extern "C" void pendSvHandler_C(void* _stackFrame)
{
  StackFrame1* frame = (StackFrame1*)_stackFrame;

  volatile int i;
  i = 1;
}
