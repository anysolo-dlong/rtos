#include "cstr.h"


namespace StdEm {
namespace Cstr {

int strLen(const char* str)
{
  int i = 0;
  for(; str[i] != 0; ++i) ;

  return i;
}

char* strDup(const char* src)
{
  const int srcLen = strLen(src);
  char* newStr = new char[srcLen+1];

  for(int i = 0; i <= srcLen; ++i)
    newStr[i] = src[i];

  return newStr;
}


}} // Cstr, StdEm
