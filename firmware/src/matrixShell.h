#ifndef MATRIX_SHELL_H_
#define MATRIX_SHELL_H_

#include <stdint.h>
#include <stdbool.h>
#include "McuShell.h"

uint8_t MATRIX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

/* needed by matrixShell.c PrintStepperStatus() */
uint8_t MATRIX_GetAddress(int32_t x, int32_t y, int32_t z);
uint8_t MATRIX_GetPos(int32_t x, int32_t y, int32_t z);

#endif /* MATRIX_SHELL_H_ */