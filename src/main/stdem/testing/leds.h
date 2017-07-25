#ifndef __STDEM_TESTING_LEDS_H__
#define __STDEM_TESTING_LEDS_H__

#include <stdem/base.h>
#include <stdem/testing/leds_port.h>


namespace StdEm {
namespace Testing {

class Leds
{
  uint32_t m_state;

public:
  Leds() {m_state = 0;}

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
  constexpr static int amountOfLeds = Port::amountOfLeds;
  void doSet(int n, bool on);
};

}} // Testing, StdEm


#endif // __STDEM_TESTING_LEDS_H__
