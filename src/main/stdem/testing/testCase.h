#ifndef __STDEM_TESTING_TESTCASE_H__
#define __STDEM_TESTING_TESTCASE_H__

#include <functional>

#include <stdem/cstr.h>
#include <stdem/assert.h>
#include <stdem/testing/logger.h>


namespace StdEm {
namespace Testing {


using TestFunc = std::function<void(void)>;


class TestCase;
class TestRunner;


class TestMethod
{
  friend class TestCase;

  TestMethod*     m_next;
  char*           m_name;
  TestFunc        m_func;

public:
  TestMethod(const char* name, TestFunc func);
  ~TestMethod();

  const char* name() const {return m_name;}
  void call() const        {m_func();}

  const TestMethod* next() const {return m_next;}
};


class TestCase
{
  friend class TestRunner;

  char*         m_name;
  TestMethod*   m_firstMethod;
  TestMethod*   m_lastMethod;
  TestCase*     m_nextCase;
  Logger*       m_logger;

public:
  TestCase(const char* name);
  virtual ~TestCase();

  StdEm::Testing::Logger& logger() {STDEM_ASSERT(m_logger != 0); return *m_logger;}

  void addTest(const char* name, TestFunc testFunc);

  const TestMethod* firstMethod() const {return m_firstMethod;}
  TestCase* nextCase() const            {return m_nextCase;}

  virtual void beforeTest() {}
  virtual void beforeTestCase() {}
};


class TestRunner
{
  TestCase* m_firstCase;
  TestCase* m_lastCase;

public:
  TestRunner();

  void add(TestCase* testCase);

  TestRunner& operator<< (TestCase& testCase) {add(&testCase); return *this;}

  void run(StdEm::Testing::Logger& logger);
};

}} // Testing, StdEm


#endif // __STDEM_TESTING_TESTCASE_H__
