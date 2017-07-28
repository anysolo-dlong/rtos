#ifndef __STDEM_TESTING_LOGGER_H__
#define __STDEM_TESTING_LOGGER_H__

#include <stdem/base.h>


namespace StdEm {
namespace Testing {


class Logger {
public:
  Logger& operator<< (const char* s) {print(s); return *this;}
  Logger& operator<< (int n);

  void print(const char* str);
};

}} // Testing, StdEm


#endif // __STDEM_TESTING_LOGGER_H__
