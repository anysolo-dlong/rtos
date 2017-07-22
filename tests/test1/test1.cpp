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

struct StackFrameAddRegs {
  uint32_t
    r4, r5, r6, r7, r8, r9, r10, r11, r14
  ;

};

struct FullStackFrame {
  StackFrameAddRegs addRegs;
  StackFrame1       subFrame1;
};


void rtos_init();
extern "C" void svcHandler_C(void*);
extern "C" void* pendSvHandler_C(void* _stackFrame);

void task0(void);
void task1(void);

void rtos_startFirstTask(StackFrame1& initialFrame);


volatile int currentTask = 0;

volatile uint32_t thread0Stack[128];
volatile uint32_t thread1Stack[128];

volatile void* threadPsp[2];

volatile uint32_t tickCounter = 0;


int main(void) {
  for(volatile long i = 0; i < 3000000; i++) ;

  HAL_Init();
  SystemInit();

  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);

  BSP_LED_Toggle(LED3);

  SysTick_Config (SystemCoreClock / 1000);

  NVIC_SetPriority(PendSV_IRQn, 0xFF); // Set PendSV to lowest possible priority

  __set_PSP(uint32_t(thread0Stack) + sizeof(thread0Stack));

  __ISB();
  __set_CONTROL(0x3); // Switch to use Process Stack, unprivileged state
  __ISB();
  asm("svc #1");

  for(;;) {}
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
        BSP_LED_Toggle(LED5);
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

      if(((cnt++) % 1000) == 0) {
        BSP_LED_Toggle(LED6);
      }
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
    BSP_LED_Toggle(LED3);

  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set PendSV to pending
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
