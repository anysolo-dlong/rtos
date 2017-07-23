#include "../base/assert.h"



Assert::AssertFunc Assert::m_assertFunc = 0;

void Assert::doAssert(const char* expr, const char* fname, int line)
{
  if(m_assertFunc != 0)
    m_assertFunc(expr, fname, line);
}
