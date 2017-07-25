#ifndef __stdem_base_assert_h__
#define __stdem_base_assert_h__

#include <stdem/base.h>


namespace StdEm {

class Assert
{
  typedef void (*AssertFunc)(const char* expr, const char* fname, int line);
  static AssertFunc m_assertFunc;

public:
  static void doAssert(const char* expr, const char* fname, int line);

  static void setAssertFunc(AssertFunc func)        {m_assertFunc = func;}
  static AssertFunc getAssertFunc(AssertFunc func)  {return m_assertFunc;}
};

} // StdEm


#ifdef STDEM_DEBUG
#define STDEM_ASSERT(expr) {if(!(expr)) Assert::doAssert(#expr, __FILE__, __LINE__);}
#else
#define STDEM_ASSERT(expr) /**/
#endif

#define STDEM_CHECK(expr) {if(!(expr)) Assert::doAssert(#expr, __FILE__, __LINE__);}

#endif // __stdem_base_assert_h__
