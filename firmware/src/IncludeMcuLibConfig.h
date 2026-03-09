/*
 * Copyright (c) 2016-2020, Erich Styger
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* header file is included with -include compiler option
Instructions:
 - Remove the 'Template_' from the name and place this file into your 'src' folder.
 - Include it with the -include compiler option with:  "${ProjDirPath}/source/IncludeMcuLibConfig.h"
 - Add the following to the -I compiler option:
../McuLib
../McuLib/config
../McuLib/config/fonts
../McuLib/fonts
../McuLib/src
../McuLib/FreeRTOS/Source/include
../McuLib/FreeRTOS/Source/portable/GCC/ARM_CM4F
../McuLib/SEGGER_RTT
../McuLib/SEGGER_Sysview
../McuLib/TraceRecorder
../McuLib/TraceRecorder/config
../McuLib/TraceRecorder/include
../McuLib/TraceRecorder/streamports/Jlink_RTT/include
../McuLib/HD44780
../McuLib/FatFS
../McuLib/FatFS/source

if using a CDT Build variable pointing to the library, the following can be used instead:
${MCULIB}
${MCULIB}/config
${MCULIB}/config/fonts
${MCULIB}/fonts
${MCULIB}/src
${MCULIB}/FreeRTOS/Source/include
${MCULIB}/FreeRTOS/Source/portable/GCC/ARM_CM4F
${MCULIB}/SEGGER_RTT
${MCULIB}/SEGGER_Sysview
${MCULIB}/TraceRecorder
${MCULIB}/TraceRecorder/config
${MCULIB}/TraceRecorder/include
${MCULIB}/TraceRecorder/streamports/Jlink_RTT/include
${MCULIB}/HD44780
${MCULIB}/minIni
${MCULIB}/FatFS
${MCULIB}/FatFS/source
 */

/* For ESP32 targets:
  - place the IncludeMcuLibConfig.h into the project 'config' folder
  - copy the template file McuLib\ESP32_CMakeLists.txt and rename it to McuLib\CMakeLists.text
  - add the following to your main CMakeLists.txt, between cmake_minimum_required() and the include():
list(APPEND EXTRA_COMPONENT_DIRS "../McuLib")
  - add the following after the include():
add_compile_options(-I../config)
add_compile_options(-include "../config/IncludeMcuLibConfig.h")

  - It should look similar to this:
    cmake_minimum_required(VERSION 3.5)

    list(APPEND EXTRA_COMPONENT_DIRS "../McuLib")

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)

    add_compile_options(-I../config)
    add_compile_options(-include "../config/IncludeMcuLibConfig.h")

    project(idf-eclipse)

 */

#ifndef INCLUDEMCULIBCONFIG_H_
#define INCLUDEMCULIBCONFIG_H_

#include "platform.h"

/* ------------------- SDK/Library ---------------------------*/
#define McuLib_CONFIG_SDK_VERSION_USED  McuLib_CONFIG_SDK_MCUXPRESSO_2_0

/* set the CPU. See McuLibConfig.h for all supported CPUs */
#define McuLib_CONFIG_CPU_IS_LPC        (1)  /* LPC family */
#define McuLib_CONFIG_CORTEX_M          (0)    /*!< 0: Cortex-M0, 3: M3, 4: M4, 7: M7, 33: M33, -1 otherwise */
#define McuLib_CONFIG_CPU_VARIANT       (McuLib_CONFIG_CPU_VARIANT_NXP_LPC845) /* for LPC need to specify the actual device */

/* ------------------- RTOS ---------------------------*/
#define McuLib_CONFIG_SDK_USE_FREERTOS       (1)
#define configTOTAL_HEAP_SIZE                (11*1024)
#define configMINIMAL_STACK_SIZE             (200/sizeof(StackType_t))
#define configTIMER_TASK_STACK_DEPTH         (400/sizeof(StackType_t))
#define configUSE_TIMERS                     (0)
#define INCLUDE_xTimerPendFunctionCall       (0)
#define configUSE_TRACE_FACILITY             (0)
#define configGENERATE_RUN_TIME_STATS        (0)

/* Shell */
#define McuShell_CONFIG_PROJECT_NAME_STRING           "LPC845-BRK ClockClock"
#define McuShell_MULTI_CMD_ENABLED                    (0)
#define McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE     (96)
/* -------------------------------------------------*/

/* McuUart485 */
#define McuUart485_CONFIG_USE_RS_485         (1)

#if PL_CONFIG_IS_MASTER
  #define McuShellUart_CONFIG_UART  McuShellUart_CONFIG_UART_LPC845_USART0
#endif

#endif /* INCLUDEMCULIBCONFIG_H_ */
