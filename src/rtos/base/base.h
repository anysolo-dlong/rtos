#ifndef __rtos_base_base_h__
#define __rtos_base_base_h__


#include <cstdint>

namespace Rtos {
namespace Base {

using Byte = uint8_t;

template <class T>
T min(T n1, T n2)
{
  if(n1 <= n2)
    return n1;
  else
    return n2;
}

template <class T>
T max(T n1, T n2)
{
  if(n1 <= n2)
    return n2;
  else
    return n1;
}

}} // Base, Rtos


#endif // __rtos_base_base_h__
