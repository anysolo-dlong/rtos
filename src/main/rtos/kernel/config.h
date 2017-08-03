#ifndef __RTOS_KERNEL_CONFIG_H__
#define __RTOS_KERNEL_CONFIG_H__


#include <rtos/kernel/base.h>
#include <stdem/list.h>


namespace Rtos {
namespace Kernel {
namespace Config {
  constexpr int ticksPerContextSwitch = 10;
}}} // Config, Kernel, Rtos

#endif // __RTOS_KERNEL_CONFIG_H__
