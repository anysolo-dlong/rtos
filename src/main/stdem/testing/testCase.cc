#include <stdem/testing/testCase.h>
#include <stdem/assert.h>


namespace StdEm {
namespace Testing {


TestMethod::Num TestMethod::m_prevNum = 0;

TestMethod::TestMethod(const char* name, TestFunc func):
  m_name(0)
{
  m_func = func;
  m_next = 0;
  m_num = ++m_prevNum;

  m_name = Cstr::strDup(name);
}


TestMethod::~TestMethod()
{
  if(m_name != 0) {
    delete[] m_name;
    m_name = 0;
  }
}


TestCase::TestCase(const char* name)
{
  m_nextCase = 0;
  m_logger = 0;
  m_name = Cstr::strDup(name);
  m_firstMethod = 0;
  m_lastMethod  = 0;
}

TestCase::~TestCase()
  {delete [] m_name;}


void TestCase::addTest(const char* name, TestFunc testFunc)
{
  TestMethod* testMethod = new TestMethod(name, testFunc);

  if(m_firstMethod == 0)
  {
    m_firstMethod = testMethod;
    m_lastMethod = testMethod;
  }
  else
  {
    m_lastMethod->m_next = testMethod;
    m_lastMethod = testMethod;
  }
}

struct RunnerState
{
  constexpr static uint32_t expectedMagic = 10295473;

  uint32_t m_magic;

  TestMethod::Num
    m_lastNum,
    m_processed,
    m_passed
  ;

  bool isInited() {return m_magic == expectedMagic;}

  void init()
  {
    m_lastNum   = -1;
    m_processed = 0;
    m_passed    = 0;
    m_magic     = expectedMagic;
  }
};

RunnerState runnerState __attribute__ ((section (".noinit")));

TestRunner::TestRunner()
{
  m_firstCase = m_lastCase  = 0;
}

void TestRunner::add(TestCase* testCase)
{
  if(m_firstCase == 0)
  {
    m_firstCase = testCase;
    m_lastCase = testCase;
  }
  else
  {
    m_lastCase->m_nextCase = testCase;
    m_lastCase = testCase;
  }
}

inline void printTestName(Logger& logger, const TestCase* testCase, const TestMethod* testMethod)
{
  logger << testCase->name() << ":" << testMethod->name();
}

bool TestRunner::findNextTest(const TestMethod*& foundMethod, TestCase*& foundCase)
{
  for(TestCase* testCase = m_firstCase; testCase != 0; testCase = testCase->nextCase())
  {
    for(const TestMethod* method = testCase->firstMethod(); method != 0; method = method->next())
    {
      if(method->num() > runnerState.m_lastNum) {
        foundMethod = method;
        foundCase = testCase;
        return true;
      }
    }
  }

  return false;
}

void TestRunner::run(Logger& logger)
{
  if(!runnerState.isInited())
    runnerState.init();

  TestCase* testCase;
  const TestMethod* method;

  if(findNextTest(method, testCase))
  {
    testCase->m_logger = &logger;

    try
    {
      printTestName(logger, testCase, method);
      logger << "...\n";

      testCase->beforeTest();
      testCase->beforeTestCase();

      method->call();

      ++runnerState.m_passed;

      printTestName(logger, testCase, method);
      logger << " PASS\n";
    }
    catch(const std::exception& e)
    {
      printTestName(logger, testCase, method);
      logger << " FAIL\n";
    }

    ++runnerState.m_processed;
    runnerState.m_lastNum = method->num();
    testCase->m_logger = 0;

    for(volatile long i = 0; i < 3000000; i++) ;
    hardwareReset();
  }
  else
  {
    logger
      << "processed tests: " << runnerState.m_processed << "\n"
      << "passed: " << runnerState.m_passed << "\n"
      << "failed: " << runnerState.m_processed - runnerState.m_passed << "\n"
    ;

    runnerState.init();
  }
}

}} // Testing, StdEm
