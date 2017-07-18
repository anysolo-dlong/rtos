cmake_minimum_required(VERSION 3.7)

project(rtos)

enable_language(ASM)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-c++)
set(CMAKE_ASM_COMPILER arm-none-eabi-as )

set(debugOpts "-g -O0")
#set(debugOpts "-O3")

set(cppOpts "-std=c++11")

set(mcuFlags "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fsingle-precision-constant -finline-functions -Wdouble-promotion" )
set(general_cflags "-Wall -ffunction-sections -fdata-sections" )
set(def_cflags "-DSTM32F4XX -DSTM32F40_41xxx -DHSE_VALUE=8000000 -D__FPU_PRESENT=1 -D__FPU_USED=1 -DARM_MATH_CM4" )

set(CMAKE_C_FLAGS "${mcuFlags} ${general_cflags} ${def_cflags} ${debugOpts}" )
set(CMAKE_CXX_FLAGS "${mcuFlags} ${general_cflags} ${def_cflags} ${debugOpts} ${cppOpts}" )

set(linkerScript "Utilities/stm32_flash.ld")


set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY lib )
set(CMAKE_INCLUDE_CURRENT_DIR ON)


include_directories( extLibs )


file(GLOB_RECURSE mclibSources anysolo/mclib/*.c*)
add_library(mclib ${mclibSources})

#set(firmwareElfFname "hwc.elf")
#set(firmwareBinFname "hwc.bin")

#file(GLOB hwcSourceFiles ${PROJECT_SOURCE_DIR}/*.s ${PROJECT_SOURCE_DIR}/*.c* Libraries/syscall/syscalls.c hardware/*.c* hardware/*.s olibs/*.c* )
#add_executable(${firmwareElfFname} ${hwcSourceFiles})
#target_include_directories( ${firmwareElfFname} PUBLIC Libraries/STM32F4xx_StdPeriph_Driver/inc )
#target_link_libraries(${firmwareElfFname} stdPeriph m c gcc)
#set_target_properties(${firmwareElfFname} PROPERTIES LINK_FLAGS "${def_cflags} -u _scanf_float -u _printf_float -Wl,--gc-sections,-T${linkerScript},-Map,map.map" )

#add_custom_command(TARGET ${firmwareElfFname} POST_BUILD
#  COMMAND arm-none-eabi-objcopy -O binary binary/${firmwareElfFname} binary/${firmwareBinFname}
#)
