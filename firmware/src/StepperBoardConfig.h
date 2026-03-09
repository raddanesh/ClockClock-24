#ifndef STEPPERBOARDCONFIG_H_
#define STEPPERBOARDCONFIG_H_

#include "McuLibconfig.h"

#define STEPBOARD_NOF_CLOCKS        (6)    /* number of clocks on a board */

#ifndef STEPBOARD_CONFIG_USE_FREERTOS_HEAP
  #define STEPBOARD_CONFIG_USE_FREERTOS_HEAP   (McuLib_CONFIG_SDK_USE_FREERTOS)
    /*!< 1: use FreeRTOS Heap (default), 0: use stdlib malloc() and free() */
#endif

#endif /* STEPPERBOARDCONFIG_H_ */
