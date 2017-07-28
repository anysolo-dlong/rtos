#ifndef __STDEM_CSTR_H__
#define __STDEM_CSTR_H__

#include <stdem/assert.h>
#include <stdem/testing/leds_port.h>


namespace StdEm {
namespace Cstr {

constexpr const char* digits = "0123456789ABCDEF";

template <class T>
char* integerToCstr(T n, char* buf, int bufSize, int radix = 10)
{
  STDEM_ASSERT(radix >= 0 && radix <= 16);

  buf[bufSize-1] = 0;
  int i = bufSize-2;
  do
  {
    STDEM_ASSERT(i >= 0);
    int d = n % radix;
    buf[i] = digits[d];

    --i;
    n /= radix;
  }while(n != 0);

  return buf + i + 1;
}

int strLen(const char* str);
char* strDup(const char*);

}} // Cstr, StdEm

#endif // __STDEM_CSTR_H__
