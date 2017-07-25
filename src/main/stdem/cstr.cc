#include "cstr.h"


namespace StdEm {
namespace Cstr {

int strLen(const char* str)
{
  int i = 0;
  for(; str[i] != 0; ++i) ;

  return i;
}

}} // Cstr, StdEm
