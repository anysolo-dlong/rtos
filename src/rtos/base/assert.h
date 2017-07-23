#ifndef _anysolo_mclib_assert_h_
#define _anysolo_mclib_assert_h_

#include "../base/base.h"

#define O_DEBUG


class Assert
{
  typedef void (*AssertFunc)(const char* expr, const char* fname, int line);
  static AssertFunc m_assertFunc;

public:
  static void doAssert(const char* expr, const char* fname, int line);

  static void setAssertFunc(AssertFunc func)        {m_assertFunc = func;}
  static AssertFunc getAssertFunc(AssertFunc func)  {return m_assertFunc;}
};


#ifdef O_DEBUG
#define O_ASSERT(expr) {if(!(expr)) Assert::doAssert(#expr, __FILE__, __LINE__);}
#else
#define O_ASSERT(expr) /**/
#endif

#define O_CHECK(expr) {if(!(expr)) Assert::doAssert(#expr, __FILE__, __LINE__);}

#endif // _anysolo_mclib_assert_h_
