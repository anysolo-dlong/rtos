cmake_minimum_required(VERSION 3.7)

project(rtos)

enable_language(ASM)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-c++)
set(CMAKE_ASM_COMPILER arm-none-eabi-as )

set(debugOpts "-g -O0 -DRTOS_DEBUG")
#set(debugOpts "-O4")

set(cfg_activePort "stm32f4xx")

set(cppOpts "-std=c++11")

set(mcuFlags "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -finline-functions -Wdouble-promotion" )
set(general_cflags "-Wall -ffunction-sections -fdata-sections" )
set(def_cflags "-DUSE_STDPERIPH_DRIVER -DSTM32F4XX -DSTM32F40_41xxx -DSTM32F407xx -DHSE_VALUE=8000000 -DHSI_VALUE=16000000 -DARM_MATH_CM4" )

set(CMAKE_C_FLAGS "${mcuFlags} ${general_cflags} ${def_cflags} ${debugOpts}" )
set(CMAKE_CXX_FLAGS "${mcuFlags} ${general_cflags} ${def_cflags} ${debugOpts} ${cppOpts}" )
SET(ASM_OPTIONS "${mcuFlags}")

set(linkerScript "stm32_flash.ld")


set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib )
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( extLibs/CMSIS/Include extLibs/CMSIS/Device/ST/STM32F4xx/Include extLibs/STM32F4xx_StdPeriph_Driver/inc )

file(GLOB_RECURSE stdPeriphSources extLibs/STM32F4xx_StdPeriph_Driver/src/*.c* extLibs/syscalls.c)
add_library(stdPeriph ${stdPeriphSources})

file(GLOB_RECURSE stdemSources src/main/stdem/*.c* src/main/stdem/*.s src/port/${cfg_activePort}/stdem/*.c* src/port/${cfg_activePort}/stdem/*.s )
add_library(stdem ${stdemSources})
target_include_directories( stdem PUBLIC src/main src/port/${cfg_activePort} )

file(GLOB_RECURSE rtosSources src/main/rtos/*.c* src/main/rtos/*.s src/port/${cfg_activePort}/rtos/*.c* src/port/${cfg_activePort}/rtos/*.s)
add_library(rtos ${rtosSources})
target_include_directories( rtos PUBLIC src/main src/port/${cfg_activePort} )

set(test1ElfFname "test1.elf")
set(test1BinFname "test1.bin")

file(GLOB_RECURSE test1SourceFiles tests/rtos/test1.cc)
add_executable(${test1ElfFname} ${test1SourceFiles})
target_include_directories( ${test1ElfFname} PUBLIC src/port/${cfg_activePort} )
target_link_libraries(${test1ElfFname} rtos stdem stdPeriph m c gcc)
set_target_properties(${test1ElfFname} PROPERTIES LINK_FLAGS "${def_cflags} -u _scanf_float -u _printf_float -Wl,--gc-sections,-T${linkerScript},-Map,map.map" )

add_custom_command(TARGET ${test1ElfFname} POST_BUILD
  COMMAND arm-none-eabi-objcopy -O binary bin/${test1ElfFname} bin/${test1BinFname}
)
