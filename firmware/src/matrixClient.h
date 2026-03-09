#ifndef MATRIXCLIENT_H_
#define MATRIXCLIENT_H_

#include "platform.h"
#include <stdint.h>
#include <stdbool.h>
#include "stepper.h"

int MATRIX_GetClockPosition(int32_t x, int32_t y);

STEPPER_Handle_t MATRIX_GetStepper(int32_t x, int32_t y, int32_t z);

void MATRIX_SetZeroPosition(STEPPER_Handle_t *motors[], size_t nofMotors);
void MATRIX_MoveByOffset(STEPPER_Handle_t *motors[], int16_t offsets[], size_t nofMotors, uint16_t delay);

uint8_t MATRIX_STEPPER_MoveHandOnSensor(STEPPER_Handle_t stepper, bool onSensor, int32_t stepSize, int32_t timeoutms, uint32_t waitms, uint16_t delay);
uint8_t MATRIX_STEPPER_ZeroHand(STEPPER_Handle_t stepper, int16_t offset, uint16_t delay);

uint8_t MATRIX_MoveHandOnSensor(STEPPER_Handle_t *motors[], size_t nofMotors, bool onSensor, int32_t stepSize, int32_t timeoutms, uint32_t waitms, uint16_t delay);
uint8_t MATRIX_ZeroHand(STEPPER_Handle_t *motors[], int16_t offsets[], size_t nofMotors, uint16_t delay);

uint8_t MATRIX_AllHandsOff(int32_t z);

uint8_t MATRIX_ZeroAllHands(void);
uint8_t MATRIX_MoveAllto(int32_t timeoutMs, int32_t degree);
uint8_t MATRIX_MoveAllto12(int32_t timeoutMs, const McuShell_StdIOType *io);

uint8_t MATRIX_ZeroBoardHands();
uint8_t MATRIX_ZeroClock(int32_t x, int32_t y, const McuShell_StdIOType *io);
uint8_t MATRIX_SetOffsetSteppersFrom12(int32_t x, int32_t y, int32_t z, const McuShell_StdIOType *io);

uint8_t MATRIX_SetBacklashSteps(int32_t x, int32_t y, int32_t z, int16_t b);

uint8_t MATRIX_SetOffset(int32_t x, int32_t y, int32_t z, int16_t b);
int16_t MATRIX_GetOffset(int32_t x, int32_t y, int32_t z);

uint8_t MATRIX_SetStepper12(int32_t x, int32_t y, int32_t z, int32_t offset);
uint8_t MATRIX_SetOffsetFrom12(void);

uint8_t MATRIX_ZeroMatrixByMid();

int32_t X12_GetBacklashSteps(void *dev, Coord_t coord, int steps);
void X12_SingleStep(void *dev, int step);

#endif /* MATRIXCLIENT_H_ */
