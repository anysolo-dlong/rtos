#ifndef __rtos_base_testLeds_h__
#define __rtos_base_testLeds_h__

#include <rtos/base/base.h>


namespace Rtos {
namespace Base {

class TestLeds
{
  uint32_t m_state;

public:
  TestLeds() {m_state = 0;}

  void toggle(int n)        {set(n, !isOn(n));}
  bool isOn(int n) const    {return (m_state & (1 << n)) != 0;}
  void set(int n, bool on)  {doSet(n, on); setState(n, on) ;}

private:
  void setState(int n, bool on)
    {
      if(on)
        m_state |= uint32_t(1) << n;
      else
        m_state &= ~(uint32_t(1) << n);
    }

// ================= should be implemented for particular board =======================
  const static int amount;
  void doSet(int n, bool on);
};

}} // Base, Rtos


#endif // __rtos_base_testLeds_h__
