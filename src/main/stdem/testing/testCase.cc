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
  m_name = Cstr::strDup(name);
  m_firstMethod = 0;
  m_lastMethod  = 0;

  m_runner = 0;
  m_logger = 0;
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

void TestCase::finishTest(bool passed)
{
  STDEM_ASSERT(m_runner != 0);
  m_runner->finishTest(passed);
}

void TestCase::runInTestContext(std::function<void(void)> code)
  { runner().runInTestContext(code); }


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

TestRunner::TestRunner(bool hardwareResetBeforeTest)
{
  m_firstCase = m_lastCase = 0;
  reset();
  m_hardwareResetBeforeTest = hardwareResetBeforeTest;
}

void TestRunner::reset()
{
  m_logger = 0;
  m_currentCase = 0;
  m_currentMethod = 0;
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
  logger << testCase->name() << " : " << testMethod->name();
}

bool TestRunner::findNextTest(TestMethod*& foundMethod, TestCase*& foundCase)
{
  for(TestCase* testCase = m_firstCase; testCase != 0; testCase = testCase->nextCase())
  {
    for(TestMethod* method = testCase->firstMethod(); method != 0; method = method->next())
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
  reset();
  m_logger = &logger;

  if(!runnerState.isInited())
    runnerState.init();

  if(findNextTest(m_currentMethod, m_currentCase))
  {
    m_currentCase->setupForRunner(this, &logger);

    runInTestContext([this] {
      printTestName(*m_logger, m_currentCase, m_currentMethod);
      *m_logger << "...\n";

      m_currentCase->beforeTest();
      m_currentCase->beforeTestCase();

      m_currentMethod->call();
    });
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

void TestRunner::runInTestContext(std::function<void(void)> code)
{
  logger() << "runInTestContext()\n";

  try
  {
    code();
    finishTest(true);
  }
  catch(const AssertionEx& e)
  {
    logger() << "ERROR: assertion \"" << e.assertion() << "\" failed.";
    if(e.text()[0] != 0)
      logger() << " " << e.text() << "\n";

    finishTest(false);
  }
  catch(const std::exception& e)
  {
    logger() << "ERROR: unexpected exception: " << e.what() << "\n";
    finishTest(false);
  }
}


void TestRunner::finishTest(bool passed)
{
  ++runnerState.m_processed;
  runnerState.m_lastNum = m_currentMethod->num();
  m_currentCase->setupForRunner(0, 0);

  printTestName(logger(), m_currentCase, m_currentMethod);

  if(passed) {
    ++runnerState.m_passed;
    logger() << " PASS\n";
  }
  else
    logger() << " FAIL\n";
  ;

  for(volatile long i = 0; i < 3000000; i++) ;
  if(m_hardwareResetBeforeTest)
    hardwareReset();
}

}} // Testing, StdEm
