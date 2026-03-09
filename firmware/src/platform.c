#include "platform.h"
#include "McuLib.h"
#include "McuRTOS.h"
#include "McuWait.h"
#include "McuUtility.h"
#include "McuGPIO.h"

#if PL_CONFIG_USE_X12_STEPPER
  #include "McuX12_017.h"
#endif
#if PL_CONFIG_USE_STEPPER
  #include "stepper.h"
#endif
#if PL_CONFIG_USE_MAG_SENSOR
  #include "magnets.h"
#endif
#if PL_CONFIG_USE_WDT
  #include "watchdog.h"
#endif
#if PL_CONFIG_USE_SHELL
  #include "Shell.h"
#endif
#if PL_CONFIG_USE_SHELL_UART
  #include "McuShellUart.h"
#endif
#if PL_CONFIG_USE_RS485
  #include "rs485.h"
#endif
#include "nvmc.h"
#if PL_CONFIG_USE_MATRIX
  #include "matrix.h"
#endif

void PL_Init(void) {
	McuLib_Init();
	McuRTOS_Init();
	McuWait_Init();
	McuUtility_Init();
	McuGPIO_Init();

#if PL_CONFIG_USE_SHELL_UART
  McuShellUart_Init();
#endif
#if PL_CONFIG_USE_SHELL
  SHELL_Init();
#endif
#if PL_CONFIG_USE_X12_STEPPER
  McuX12_017_Init();
#endif
#if PL_CONFIG_USE_STEPPER
  STEPPER_Init();
#endif
#if PL_CONFIG_USE_MAG_SENSOR
  MAG_Init();
#endif
#if PL_CONFIG_USE_RS485
  RS485_Init();
#endif
  NVMC_Init();
#if PL_CONFIG_USE_MATRIX
  MATRIX_Init();
#endif
#if PL_CONFIG_USE_WDT
  WDT_Init();
#endif
}

