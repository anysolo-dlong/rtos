#include <rtos/kernel/scheduler.h>
#include <rtos/kernel/config.h>
#include <stdem/testing/leds.h>

#include <cstring>

#include "stm32f4xx.h"
#include "core_cm4.h"


namespace Rtos {
namespace Kernel {

StdEm::Testing::Leds leds;


Tcb::Tcb(ThreadFunc func, void* arg, void* stackData, uint32_t stackSize, Prio prio):
    m_allNode(this), m_activeNode(this), m_sleepNode(this)
{
  STDEM_ASSERT((stackSize % 8) == 0);

  m_func        = func;
  m_funcArg     = arg;
  m_stackBegin  = stackData;
  m_stackSize   = stackSize;
  m_prio        = prio;

  m_sp = 0;
  m_wakeupTime = 0;
}

void Tcb::init(StackFrame1& initialFrame)
{
  m_sp = (FullStackFrame*)(uint32_t(m_stackBegin) + m_stackSize - sizeof(FullStackFrame));
  FullStackFrame* frame = (FullStackFrame*)m_sp;

  m_wakeupTime = 0;

  frame->m_subFrame1 = initialFrame;
  frame->m_subFrame1.pc = RegData(m_func);
  frame->m_subFrame1.r0 = RegData(m_funcArg);
}


Scheduler scheduler;

inline void initiateContextSwitching()
{
  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Set PendSV to pending
}

Scheduler::Scheduler()
{
  m_currentThread     = 0;
  m_tickCounter       = 0;
  m_ticksBeforeSwitch = 0;
}

void Scheduler::start()
{
  NVIC_SetPriority(PendSV_IRQn, 0xFF); // Set PendSV to lowest possible priority
  SCB->CCR |= SCB_CCR_STKALIGN_Msk;

  Tcb* firstThread = m_activeThreads(m_activeThreads.head());
  __set_PSP(uint32_t(firstThread->m_stackBegin) + firstThread->m_stackSize);
  __ISB();

  __set_CONTROL(0x3); // Switch to use Process Stack, unprivileged state
  __ISB();

  asm("svc #1");
  for(;;) ;
}


void Scheduler::fiu_startFirstTask(StackFrame1& initialFrame)
{
  for(ListNode* nd = m_activeThreads.head(); nd != 0; nd = nd->next())
  {
    Tcb& tcb = m_activeThreads[nd];

    if(m_currentThread == 0)
      m_currentThread = &tcb;

    tcb.init(initialFrame);
  }

  SysTick_Config (SystemCoreClock / 1000);
}

void Scheduler::addThread(Tcb* tcb)
{
  m_allThreads.insertTail(&tcb->m_allNode);
  m_activeThreads.insertTail(&tcb->m_activeNode);
}

void Scheduler::activateThread(Tcb* tcb)
{
  insertWithPrio(m_activeThreads, &tcb->m_activeNode);
}

void Scheduler::insertWithPrio(TcbList& list, ListNode* newNode)
{
  for(ListNode* nd = list.head(); nd != 0; nd = nd->next())
  {
    if(list[newNode].m_prio < list[nd].m_prio)
    {
      list.insertBefore(newNode, nd);
      return;
    }
  }

  list.insertTail(newNode);
}

void Scheduler::unsleepThreads()
{
  for(ListNode* nd = m_sleepThreads.head(); nd != 0;)
  {
    Tcb& tcb = m_sleepThreads[nd];

    if(tcb.m_wakeupTime <= tickCounter())
    {
      tcb.m_wakeupTime = 0;
      ListNode* nextNode = nd->next();

      tcb.m_wakeupTime = 0;
//      tcb.m_modes &= ~Tcb::Modes(Tcb::mSleep);
      m_sleepThreads.remove(nd);
      activateThread(&tcb);

      nd = nextNode;
    }
    else // rest threads have more bigger wakeTime
      break;
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

  switch(svcArg)
  {
    case 1:
      scheduler.fiu_startFirstTask(*frame);
      break;

    case 2:
      NVIC_SystemReset();
      break;

    case 3:
      SysTick->CTRL  &= uint32_t(SysTick_CTRL_ENABLE_Msk);
      break;
  }
}

extern "C" void SysTick_Handler()
{
  scheduler.fiu_tick();
}

void Scheduler::fiu_tick()
{
  ++m_tickCounter;

  if((m_tickCounter % 500) == 0)
    leds.toggle(3);

  if(m_ticksBeforeSwitch == 0)
  {
    m_ticksBeforeSwitch = Config::ticksPerContextSwitch-1;
    initiateContextSwitching();
  }
  else
    --m_ticksBeforeSwitch;
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

extern "C" void* pendSvHandler_C(void* stackFrame)
{
  return scheduler.fiu_switchContext(stackFrame);
}

void* Scheduler::fiu_switchContext(void* currentThreadStack)
{
  STDEM_ASSERT(!m_activeThreads.isEmpty());

  if(!m_activeThreads.isEmpty())
  {
    if(m_currentThread != 0)
    {
      m_currentThread->m_sp = (FullStackFrame*)currentThreadStack;

      // move to the end of the queue of the same priority
      m_activeThreads.remove(& m_currentThread->m_activeNode);
      insertWithPrio(m_activeThreads, &m_currentThread->m_activeNode);
    }

    m_currentThread = m_activeThreads(m_activeThreads.head());
  }

  return m_currentThread->m_sp;
}

#if 0
void Scheduler::sleepThread(Tcb* tcb, Ticks sleepInTicks)
{
  Word cpsr = oeosDisableInterrupts();

//  m_activeThreads.remove(& tcb->m_activeNode);

  tcb->m_wakeupTime = tickCounter() + sleepInTicks;
  tcb->m_modes |= Tcb::mGonnaSleep;
//  insertInSleepThreads(tcb);
  oeosEnableInterrupts(cpsr);

  oeosSwiCall();

}

void Scheduler::insertInSleepThreads(Tcb* tcb)
{
  for(ListNode* nd = m_sleepThreads.head(); nd != 0; nd = nd->next())
  {
    if(tcb->m_wakeupTime < m_sleepThreads[nd].m_wakeupTime)
    {
      m_sleepThreads.insertBefore(& tcb->m_sleepNode, nd);
      return;
    }
  }

  m_sleepThreads.insertTail(& tcb->m_sleepNode);
}


#endif

}} // Kernel, Rtos
