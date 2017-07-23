#ifndef __rtos_base_assert_h__
#define __rtos_base_assert_h__

#include <rtos/base/base.h>


namespace Rtos {
namespace Base {

class Assert
{
  typedef void (*AssertFunc)(const char* expr, const char* fname, int line);
  static AssertFunc m_assertFunc;

public:
  static void doAssert(const char* expr, const char* fname, int line);

  static void setAssertFunc(AssertFunc func)        {m_assertFunc = func;}
  static AssertFunc getAssertFunc(AssertFunc func)  {return m_assertFunc;}
};

}} // Base, Rtos


#ifdef RTOS_DEBUG
#define RTOS_ASSERT(expr) {if(!(expr)) Assert::doAssert(#expr, __FILE__, __LINE__);}
#else
#define RTOS_ASSERT(expr) /**/
#endif

#define RTOS_CHECK(expr) {if(!(expr)) Assert::doAssert(#expr, __FILE__, __LINE__);}

#endif // __rtos_base_assert_h__
