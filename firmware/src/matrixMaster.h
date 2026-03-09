#ifndef MATRIXMASTER_H_
#define MATRIXMASTER_H_

#include "platform.h"
#if PL_CONFIG_USE_X12_STEPPER
  #include "McuX12_017.h"
#endif
#if PL_CONFIG_IS_MASTER
  #include "matrixconfig.h"
#endif

MatrixClock_t MATRIX_GetClock(uint8_t x, uint8_t y);

STEPPER_Handle_t MATRIX_GetStepper(int32_t x, int32_t y, int32_t z);

const unsigned char*GetModeString(STEPPER_MoveMode_e mode, bool speedUp, bool slowDown);

void MATRIX_CopyMatrix(void);

void MATRIX_Delay(int32_t ms);

bool MATRIX_BoardIsEnabled(uint8_t addr);

uint8_t MATRIX_CheckRemoteLastError(void);

void QueueMoveCommand(int x, int y, int z, int angle, int delay, STEPPER_MoveMode_e mode, bool speedUp, bool slowDown, bool absolute);

uint8_t MATRIX_ExecuteRemoteQueue(void);
uint8_t MATRIX_SendToRemoteQueue(void);

uint8_t MATRIX_ExecuteRemoteQueueAndWait(const McuShell_StdIOType *io);
uint8_t MATRIX_SendToRemoteQueueExecuteAndWait(const McuShell_StdIOType *io);

uint8_t MATRIX_DrawClockDelays(uint8_t x, uint8_t y, uint8_t delay0, uint8_t delay1);
uint8_t MATRIX_DrawClockHands(uint8_t x, uint8_t y, int16_t angle0, int16_t angle1);
uint8_t MATRIX_DrawAllClockHands(int16_t angle0, int16_t angle1);
uint8_t MATRIX_DrawMoveMode(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1);
uint8_t MATRIX_DrawAllMoveMode(STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1);


void DrawNumber(MClockDigit_t *digit, uint8_t xPos, uint8_t yPos);
uint8_t MATRIX_WaitForIdle(int32_t timeoutMs);


uint8_t MATRIX_ShowTime(uint8_t hour, uint8_t minute);
uint8_t MATRIX_ShowDigit(uint8_t digit);
uint8_t MATRIX_ShowTemperature(uint8_t temperature);


void MATRIX_WriteNumber(const McuShell_StdIOType *io);
uint8_t MATRIX_MoveAllto12(int32_t timeoutMs, const McuShell_StdIOType *io);

uint8_t MATRIX_DrawAllClockDelays(uint8_t delay0, uint8_t delay1);


#endif /* MATRIXMASTER_H_ */
