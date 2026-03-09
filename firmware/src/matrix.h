#ifndef MATRIX_H_
#define MATRIX_H_

#include "platform.h"
#if PL_CONFIG_IS_MASTER
  #include "matrixconfig.h"
#endif
#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"
#include "stepper.h"


uint8_t MATRIX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

void MATRIX_TimerCallback(void);

void MATRIX_Init(void);

#endif /* MATRIX_H_ */
