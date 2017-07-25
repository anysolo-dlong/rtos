#ifndef __STDEM_TESTING_LOGGER_H__
#define __STDEM_TESTING_LOGGER_H__

#include "../cstr.h"


namespace StdEm {
namespace Testing {


class TestLogger {
public:
  TestLogger& operator<< (const char* s) {print(s);}
  TestLogger& operator<< (int n);

  void print(const char* str);
};

}} // Testing, StdEm


#endif // __STDEM_TESTING_LOGGER_H__
