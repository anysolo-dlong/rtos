#ifndef __STDEM_TESTING_TESTCASE_H__
#define __STDEM_TESTING_TESTCASE_H__

#include <functional>

#include <stdem/cstr.h>
#include <stdem/assert.h>
#include <stdem/testing/logger.h>


namespace StdEm {
namespace Testing {


using TestFunc = std::function<void(void)>;


class AssertionEx: public std::exception
{
  const char* m_assertion;
  const char* m_text;

public:
  AssertionEx(const char* assertion, const char* text): m_assertion(assertion), m_text(text) {}

  const char* assertion() const {return m_assertion;}
  const char* text() const      {return m_text;}
};


namespace Assertions {
  inline void isTrue(bool val, const char* text) {
    if (!val)
      throw AssertionEx("isTrue", text);
  }

  inline void isFalse(bool val, const char* text) {
    if (val)
      throw AssertionEx("isFalse", text);
  }
}

class TestCase;
class TestRunner;


class TestMethod
{
  friend class TestCase;

public:
  using Num = int32_t;

private:
  TestMethod*     m_next;
  char*           m_name;
  TestFunc        m_func;
  Num             m_num;

  static Num       m_prevNum;

public:
  TestMethod(const char* name, TestFunc func);
  ~TestMethod();

  const char* name() const {return m_name;}
  Num num() const          {return m_num;}
  void call() const        {m_func();}

  TestMethod* next() const {return m_next;}
};


class TestRunner;

class TestCase
{
  friend class TestRunner;

  char*         m_name;
  TestMethod*   m_firstMethod;
  TestMethod*   m_lastMethod;
  TestCase*     m_nextCase;

  TestRunner*   m_runner;
  Logger*       m_logger;

  void setupForRunner(TestRunner* runner, Logger* logger)
    {m_runner = runner; m_logger = logger;}

public:
  explicit TestCase(const char* name);
  virtual ~TestCase();

  StdEm::Testing::Logger& logger() {STDEM_ASSERT(m_logger != 0); return *m_logger;}

  const char* name() const {return m_name;}

  void addTest(const char* name, TestFunc testFunc);

  TestMethod* firstMethod() {return m_firstMethod;}
  TestCase* nextCase()      {return m_nextCase;}

  virtual void beforeTest() {}
  virtual void beforeTestCase() {}

  void finishTest(bool passed);
  void runInTestContext(std::function<void(void)> code);

  TestRunner& runner() const {return *m_runner;}
};


class TestRunner
{
  bool      m_hardwareResetBeforeTest;
  TestCase* m_firstCase;
  TestCase* m_lastCase;

  StdEm::Testing::Logger*   m_logger;
  TestCase*                 m_currentCase;
  TestMethod*               m_currentMethod;

public:
  TestRunner(bool hardwareResetBeforeTest = true);
  virtual ~TestRunner() {}

  void add(TestCase* testCase);
  void finishTest(bool passed);

  TestRunner& operator<< (TestCase& testCase) {add(&testCase); return *this;}

  void run(StdEm::Testing::Logger& logger);
  void runInTestContext(std::function<void(void)> code);

  Logger& logger() const {STDEM_ASSERT(m_logger != 0); return *m_logger;}

protected:
  virtual void hardwareReset();

private:
  void reset();
  bool findNextTest(TestMethod*& foundMethod, TestCase*& foundCase);
};

}} // Testing, StdEm


#endif // __STDEM_TESTING_TESTCASE_H__
