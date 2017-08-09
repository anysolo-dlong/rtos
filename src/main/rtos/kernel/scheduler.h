#ifndef __RTOS_KERNEL_SCHDULER_H__
#define __RTOS_KERNEL_SCHDULER_H__


#include <rtos/kernel/base.h>
#include <stdem/list.h>


namespace Rtos {
namespace Kernel {

using Prio = int8_t;
using Tick = uint32_t;
using RegData = uint32_t;
using ThreadFunc = void (*)(void*);


struct StackFrame1
{
  RegData
    r0, r1, r2, r3,
    r12, lr, pc, xpsr
  ;
};

struct StackFrameAddRegs {
  // TODO: inverse the order of the registers here?
  RegData
    r4, r5, r6, r7, r8, r9, r10, r11, r14
  ;

};

struct StackFrameFpuRegs {
  RegData s[16];
};

// todo: probably we do not need to know exact layout or the stack frames. Thus these structs contains too many detail.

struct FullStackFrame {
  StackFrameAddRegs m_addRegs;
  StackFrame1       m_subFrame1;
};

struct FullStackFpuFrame {
  StackFrameAddRegs m_addRegs;
  StackFrameFpuRegs m_fpu;
  StackFrame1       m_subFrame1;
};


struct Tcb
{
  ThreadFunc  m_func;
  void*       m_funcArg;
  void*       m_stackBegin;
  uint32_t    m_stackSize;
  Prio        m_prio;

  StdEm::ListNode   m_allNode, m_activeNode, m_sleepNode;
  FullStackFrame*   m_sp;
  Tick              m_wakeupTime; // time for wake up in ticks, only for sleeping threads

  Tcb(ThreadFunc func, void* arg, void* stackData, uint32_t stackSize, Prio prio);
  void init(StackFrame1& initialFrame);
};


class Scheduler {
public:
  using TcbList = StdEm::ListT<Tcb>;
  using ListNode = StdEm::ListNode;

private:
  Tick    m_tickCounter;
  Tick    m_ticksBeforeSwitch;
  TcbList m_allThreads;
  TcbList m_activeThreads;
  TcbList m_sleepThreads;

  Tcb* m_currentThread;

  void insertWithPrio(TcbList& list, ListNode* newNode);
  void insertInSleepThreads(Tcb* tcb);
  void unsleepThreads();

public:
  Scheduler();
  void start();

  void addThread(Tcb* tcb);
  void activateThread(Tcb* tcb);

  Tick tickCounter() const {return m_tickCounter;}

  void schedule();

  // For call from Scheduler ISRs only!
  void fiu_tick();
  void* fiu_switchContext(void* currentThreadStack);
  void fiu_startFirstTask(StackFrame1& frame);

  //  void sleepThread(Tcb* tcb, Ticks sleepInTicks);
};

}}// Kernel, Rtos


#endif // __RTOS_KERNEL_SCHDULER_H__

