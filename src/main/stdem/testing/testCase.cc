#include <stdem/testing/testCase.h>
#include <stdem/assert.h>


namespace StdEm {
namespace Testing {


TestMethod::TestMethod(const char* name, TestFunc func):
  m_name(Cstr::strDup(name)),
  m_func(func),
  m_next(0)
{}

TestMethod::~TestMethod()
  {delete[] m_name;}


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

void TestRunner::run(Logger& logger)
{
  for(TestCase* testCase = m_firstCase; testCase != 0; testCase = testCase->nextCase())
  {
    logger << "testCase: " << testCase->m_name << "\n";
    for(const TestMethod* method = testCase->firstMethod(); method != 0; method = method->next())
    {
      testCase->m_logger = &logger;

      try
      {
        logger << "test: " << method->name() << "...\n";
        testCase->beforeTest();
        testCase->beforeTestCase();

        method->call();

        logger << "test: " << method->name() << " PASS\n";
      }
      catch(const std::exception& e)
      {
        logger << "test: " << method->name() << " FAIL. Ex:" << e.what() << "\n";
      }

      testCase->m_logger = 0;
    }
  }
}


}} // Testing, StdEm
