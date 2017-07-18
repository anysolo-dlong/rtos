#ifndef _olibs_base_h_
#define _olibs_base_h_

//#define O_TESTING


#include <cstdint>

namespace mclib {
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

#endif // _olibs_base_h_

} // namespace mclib
