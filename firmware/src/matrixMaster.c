#include "platform.h"
#if PL_CONFIG_IS_MASTER
#include "matrixconfig.h"
#include "matrixMaster.h"
#include "rs485.h"
#include "McuUtility.h"
#include "McuWait.h"
#include "stepper.h"
#include "Shell.h"


MatrixClock_t clockMatrix[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y] = {
		{
				[0].enabled = true,
				[0].addr = 0X0A,
				[0].nr = 1,
				[0].board.x = 0,
				[0].board.y = 0,

				[1].enabled = true,
				[1].addr = 0X0A,
				[1].nr = 2,
				[1].board.x = 0,
				[1].board.y = 1,

				[2].enabled = true,
				[2].addr = 0X0A,
				[2].nr = 4,
				[2].board.x = 0,
				[2].board.y = 2,
		},
		{
				[0].enabled = true,
				[0].addr = 0X0A,
				[0].nr = 0,
				[0].board.x = 1,
				[0].board.y = 0,

				[1].enabled = true,
				[1].addr = 0X0A,
				[1].nr = 3,
				[1].board.x = 1,
				[1].board.y = 1,

				[2].enabled = true,
				[2].addr = 0X0A,
				[2].nr = 5,
				[2].board.x = 1,
				[2].board.y = 2,
		}
};


MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS] = {
		[0].enabled = true,
		[0].addr = 0X0A
};


MATRIX_Matrix_t matrix; /* map of current matrix */
MATRIX_Matrix_t prevMatrix; /* map of previous matrix, used to reduce communication traffic */

MatrixClock_t MATRIX_GetClock(uint8_t x, uint8_t y) {
	return clockMatrix[x][y];
}

void MATRIX_CopyMatrix(void) {
  memcpy(&prevMatrix, &matrix, sizeof(MATRIX_Matrix_t));
}

const unsigned char*GetModeString(STEPPER_MoveMode_e mode, bool speedUp, bool slowDown) {
  const unsigned char *str = (unsigned char*)"SH"; /* default and error case */
  if (speedUp) {
    if (slowDown) {
      switch(mode) {
        case STEPPER_MOVE_MODE_SHORT: str = (unsigned char*)"sh";  break;
        case STEPPER_MOVE_MODE_CW:    str = (unsigned char*)"cw";  break;
        case STEPPER_MOVE_MODE_CCW:   str = (unsigned char*)"cc";  break;
      }
    } else {
      switch(mode) {
        case STEPPER_MOVE_MODE_SHORT: str = (unsigned char*)"sH";  break;
        case STEPPER_MOVE_MODE_CW:    str = (unsigned char*)"cW";  break;
        case STEPPER_MOVE_MODE_CCW:   str = (unsigned char*)"cC";  break;
      }
    }
  } else {
    if (slowDown) {
      switch(mode) {
        case STEPPER_MOVE_MODE_SHORT: str = (unsigned char*)"Sh";  break;
        case STEPPER_MOVE_MODE_CW:    str = (unsigned char*)"Cw";  break;
        case STEPPER_MOVE_MODE_CCW:   str = (unsigned char*)"Cc";  break;
      }
    } else {
      switch(mode) {
        case STEPPER_MOVE_MODE_SHORT: str = (unsigned char*)"SH";  break;
        case STEPPER_MOVE_MODE_CW:    str = (unsigned char*)"CW";  break;
        case STEPPER_MOVE_MODE_CCW:   str = (unsigned char*)"CC";  break;
      }
    }
  }
  return str;
}

MClockDigit_t clockDigits[10] = {
    [0].digit = {
        [0][0]={.hands={{180},{ 90}}}, [1][0]={.hands={{270},{180}}},
        [0][1]={.hands={{  0},{180}}}, [1][1]={.hands={{  0},{180}}},
        [0][2]={.hands={{  0},{ 90}}}, [1][2]={.hands={{270},{  0}}},
    },
    [1].digit = {
    	[0][0]={.hands={{225},{225}}}, [1][0]={.hands={{180},{180}}},
    	[0][1]={.hands={{225},{225}}}, [1][1]={.hands={{  0},{180}}},
    	[0][2]={.hands={{225},{225}}}, [1][2]={.hands={{  0},{  0}}},
    },

    [2].digit = {
        [0][0]={.hands={{ 90},{ 90}}}, [1][0]={.hands={{270},{180}}},
        [0][1]={.hands={{180},{ 90}}}, [1][1]={.hands={{270},{  0}}},
        [0][2]={.hands={{  0},{ 90}}}, [1][2]={.hands={{270},{270}}},
    },
    [3].digit = {
        [0][0]={.hands={{ 90},{ 90}}}, [1][0]={.hands={{270},{180}}},
        [0][1]={.hands={{ 90},{ 90}}}, [1][1]={.hands={{270},{  0}}},
        [0][2]={.hands={{ 90},{ 90}}}, [1][2]={.hands={{270},{  0}}},
    },
    [4].digit = {
        [0][0]={.hands={{180},{180}}}, [1][0]={.hands={{180},{180}}},
        [0][1]={.hands={{  0},{ 90}}}, [1][1]={.hands={{270},{  0}}},
        [0][2]={.hands={{225},{225}}}, [1][2]={.hands={{  0},{  0}}},
    },
    [5].digit = {
        [0][0]={.hands={{180},{ 90}}}, [1][0]={.hands={{270},{270}}},
        [0][1]={.hands={{  0},{ 90}}}, [1][1]={.hands={{270},{180}}},
        [0][2]={.hands={{ 90},{ 90}}}, [1][2]={.hands={{270},{  0}}},
    },
    [6].digit = {
        [0][0]={.hands={{180},{ 90}}}, [1][0]={.hands={{270},{270}}},
        [0][1]={.hands={{  0},{180}}}, [1][1]={.hands={{270},{180}}},
        [0][2]={.hands={{  0},{ 90}}}, [1][2]={.hands={{270},{  0}}},
    },
    [7].digit = {
        [0][0]={.hands={{ 90},{ 90}}}, [1][0]={.hands={{270},{180}}},
        [0][1]={.hands={{225},{225}}}, [1][1]={.hands={{  0},{180}}},
        [0][2]={.hands={{225},{225}}}, [1][2]={.hands={{  0},{  0}}},
    },
    [8].digit = {
        [0][0]={.hands={{180},{ 90}}}, [1][0]={.hands={{270},{180}}},
        [0][1]={.hands={{180},{ 90}}}, [1][1]={.hands={{270},{180}}},
        [0][2]={.hands={{  0},{ 90}}}, [1][2]={.hands={{270},{  0}}},
    },
    [9].digit = {
        [0][0]={.hands={{180},{ 90}}}, [1][0]={.hands={{270},{180}}},
        [0][1]={.hands={{  0},{ 90}}}, [1][1]={.hands={{  0},{180}}},
        [0][2]={.hands={{ 90},{ 90}}}, [1][2]={.hands={{270},{  0}}},
    },
};

#if PL_CONFIG_USE_STEPPER
STEPPER_Handle_t MATRIX_GetStepper(int32_t x, int32_t y, int32_t z) {
  STEPPER_Handle_t stepper;
  STEPBOARD_Handle_t board;

  if (x>=MATRIX_NOF_CLOCKS_X || y>=MATRIX_NOF_CLOCKS_Y || z>=MATRIX_NOF_CLOCKS_Z) {
    return NULL;
  }
  board = MATRIX_AddrGetBoard(clockMatrix[x][y].addr);
  if (board==NULL) {
    return NULL;
  }
  stepper = STEPBOARD_GetStepper(board, clockMatrix[x][y].nr, z);
  return stepper;
}
#endif

void MATRIX_Delay(int32_t ms) {
  while (ms>100) { /* wait in smaller pieces to keep watchdog task informed */
    vTaskDelay(pdMS_TO_TICKS(100));
#if PL_CONFIG_USE_WDT
  WDT_Report(WDT_REPORT_ID_CURR_TASK, 100);
#endif
    ms -= 100;
  }
  /* wait for the remaining time */
  vTaskDelay(pdMS_TO_TICKS(ms));
#if PL_CONFIG_USE_WDT
  WDT_Report(WDT_REPORT_ID_CURR_TASK, ms);
#endif
}

bool MATRIX_BoardIsEnabled(uint8_t addr) {
  for(int i=0; i<MATRIX_NOF_BOARDS; i++){
#if PL_CONFIG_USE_STEPPER
    if (STEPBOARD_GetAddress(MATRIX_Boards[i])==addr) {
      return STEPBOARD_IsEnabled(MATRIX_Boards[i]);
    }
#else
    if (MATRIX_BoardList[i].addr==addr) {
      return MATRIX_BoardList[i].enabled;
    }
#endif
  }
  return false;
}

uint8_t MATRIX_DrawClockHands(uint8_t x, uint8_t y, int16_t angle0, int16_t angle1) {
  if (x>=MATRIX_NOF_CLOCKS_X || y>=MATRIX_NOF_CLOCKS_Y) {
    return ERR_FRAMING;
  }
  matrix.angleMap[x][y][0] = angle0;
  matrix.angleMap[x][y][1] = angle1;
  return ERR_OK;
}

uint8_t MATRIX_DrawAllClockHands(int16_t angle0, int16_t angle1) {
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
    	matrix.angleMap[x][y][0] = angle0;
    	matrix.angleMap[x][y][1] = angle1;
    }
  }
  return ERR_OK;
}

uint8_t MATRIX_DrawMoveMode(uint8_t x, uint8_t y, STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1) {
  if (x>=MATRIX_NOF_CLOCKS_X || y>=MATRIX_NOF_CLOCKS_Y) {
    return ERR_FRAMING;
  }
  matrix.moveMap[x][y][0] = mode0;
  matrix.moveMap[x][y][1] = mode1;
  return ERR_OK;
}

uint8_t MATRIX_DrawAllMoveMode(STEPPER_MoveMode_e mode0, STEPPER_MoveMode_e mode1) {
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
    	matrix.moveMap[x][y][0] = mode0;
    	matrix.moveMap[x][y][1] = mode1;
    }
  }
  return ERR_OK;
}

void DrawNumber(MClockDigit_t *digit, uint8_t xPos, uint8_t yPos) {
  /* angle <c> <m> <deg> <md>; Move clock (0-3) and motor (0-1) to angle (0..359) using mode (ccw, cw, short)   */

  for(int y=0; y<3; y++) { /* every clock row */
    for(int x=0; x<2; x++) { /* every clock column */
    	(void)MATRIX_DrawClockHands(xPos+x, yPos+y, digit->digit[x][y].hands[0].angle, digit->digit[x][y].hands[1].angle);
    }
  }
}

uint8_t MATRIX_WaitForIdle(int32_t timeoutMs) {
  bool boardIsIdle[MATRIX_NOF_BOARDS];
  uint8_t res;
  uint8_t addr;
  bool isEnabled;

  for(int i=0; i<MATRIX_NOF_BOARDS; i++) { /* initialize array */
    boardIsIdle[i] = false;
  }
  for(;;) { /* breaks or returns */
    for(int i=0; i<MATRIX_NOF_BOARDS; i++) { /* go through all boards */
      if (!boardIsIdle[i]) { /* ask board if it is still not idle */
  #if PL_CONFIG_USE_STEPPER
        isEnabled = STEPBOARD_IsEnabled(MATRIX_Boards[i]);
        addr = STEPBOARD_GetAddress(MATRIX_Boards[i]);
  #else
        isEnabled = MATRIX_BoardList[i].enabled;
        addr = MATRIX_BoardList[i].addr;
  #endif
        if (isEnabled) {
          res = RS485_SendCommand(addr, (unsigned char*)"idle", 1000, false, 1); /* ask board if it is idle */
          if (res==ERR_OK) { /* board is idle */
            boardIsIdle[i] = true;
          }
        } else { /* board is not enabled, so we consider it as idle */
          boardIsIdle[i] = true;
        }
      }
    } /* for all boards */
    /* check if all boards are idle now */
    for(int i=0; /* breaks */; i++) {
      if (i==MATRIX_NOF_BOARDS) {
        return ERR_OK; /* all boards are idle now */
      }
      if (!boardIsIdle[i]) {
        break; /* at least one is still not idle: break this loop and check non-idle boards again */
      }
    } /* for */
    MATRIX_Delay(1000); /* give boards time to get idle */
  #if PL_CONFIG_USE_WDT
    WDT_Report(WDT_REPORT_ID_CURR_TASK, 1000);
  #endif
    timeoutMs -= 1000; /* more timeout if boards do not respond */
    if (timeoutMs<0) {
      return ERR_BUSY;
    }
  } /* for which breaks or returns */
  return ERR_OK;
}

uint8_t MATRIX_CheckRemoteLastError(void) {
    bool boardHasError[MATRIX_NOF_BOARDS];
    uint8_t res;
    uint8_t addr;
    bool isEnabled;

    for(int i=0; i<MATRIX_NOF_BOARDS; i++) {
      boardHasError[i] = false;
    }
    for(int i=0; i<MATRIX_NOF_BOARDS; i++) {
      if (!boardHasError[i]) { /* ask board if it is still not idle */
  #if PL_CONFIG_USE_STEPPER
        isEnabled = STEPBOARD_IsEnabled(MATRIX_Boards[i]);
        addr = STEPBOARD_GetAddress(MATRIX_Boards[i]);
  #else
        isEnabled = MATRIX_BoardList[i].enabled;
        addr = MATRIX_BoardList[i].addr;
  #endif
        if (isEnabled) {
          res = RS485_SendCommand(addr, (unsigned char*)"lastError", 1000, false, 1); /* ask board if there was an error */
          if (res==ERR_OK) { /* no error */
            boardHasError[i] = false;
          } else { /* send command again! */
            boardHasError[i] = true;
            (void)RS485_SendCommand(addr, (unsigned char*)"matrix exq", 1000, true, 1); /* execute the queue */
          }
        } else { /* board is not enabled, so it is considered to be fine */
          boardHasError[i] = false;
        }
      }
    } /* for */
    return ERR_OK;
}

uint8_t MATRIX_ExecuteRemoteQueue(void) {
    /* send broadcast execute queue command */
    (void)RS485_SendCommand(RS485_BROADCAST_ADDRESS, (unsigned char*)"matrix exq", 1000, true, 0); /* execute the queue */
    /* send it again, just to make sure if a board has not received it: */
    (void)RS485_SendCommand(RS485_BROADCAST_ADDRESS, (unsigned char*)"matrix exq", 1000, true, 0); /* execute the queue */
    /* check with lastEror if all have received the message */
    return MATRIX_CheckRemoteLastError();
}

void QueueMoveCommand(int x, int y, int z, int angle, int delay, STEPPER_MoveMode_e mode, bool speedUp, bool slowDown, bool absolute) {
  uint8_t buf[McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE];

  /*  matrix q <x> <y> <z> a <angle> <delay> <mode> */
  McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"matrix q ");
  McuUtility_strcatNum8u(buf, sizeof(buf), clockMatrix[x][y].board.x); /* <x> */
  McuUtility_chcat(buf, sizeof(buf), ' ');
  McuUtility_strcatNum8u(buf, sizeof(buf), clockMatrix[x][y].board.y); /* <y> */
  McuUtility_chcat(buf, sizeof(buf), ' ');
  McuUtility_strcatNum8u(buf, sizeof(buf), z); /* <z> */
  McuUtility_strcat(buf, sizeof(buf), absolute?(unsigned char*)" a ":(unsigned char*)" r ");
  McuUtility_strcatNum16u(buf, sizeof(buf), angle); /* <a> */
  McuUtility_chcat(buf, sizeof(buf), ' ');
  McuUtility_strcatNum16u(buf, sizeof(buf), delay); /* <d> */
  McuUtility_chcat(buf, sizeof(buf), ' ');
  McuUtility_strcat(buf, sizeof(buf), GetModeString(mode, speedUp, slowDown));
  (void)RS485_SendCommand(clockMatrix[x][y].addr, buf, 1000, true, 1); /* queue the command for the remote boards */
}

uint8_t MATRIX_SendToRemoteQueue(void) {
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) { /* every clock row */
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) { /* every PCB in column */
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
        if (MATRIX_BoardIsEnabled(clockMatrix[x][y].addr) && clockMatrix[x][y].enabled) { /* check if board and clock is enabled */
          if (matrix.angleMap[x][y][z]!=prevMatrix.angleMap[x][y][z]) {
            /* send only if there is a change */
            QueueMoveCommand(x, y, z, matrix.angleMap[x][y][z], matrix.delayMap[x][y][z], matrix.moveMap[x][y][z], true, true, true);
          }
        }
      }
    }
  }
  //MATRIX_CopyMatrix(&prevMatrix, &matrix); /* make backup */
  MATRIX_CopyMatrix();
  return ERR_OK;
}

uint8_t MATRIX_ExecuteRemoteQueueAndWait(const McuShell_StdIOType *io) {
  uint8_t res;

  res = MATRIX_ExecuteRemoteQueue();
  if (res!=ERR_OK) {
    //McuShell_SendStr((unsigned char*)("MATRIX_ExecuteRemoteQueueAndWait: failed executing!\r\n"), io->stdErr);
    return res;
  }
  MATRIX_Delay(500); /* give the clocks some time to start executing */
  res = MATRIX_WaitForIdle(30000);
  if (res!=ERR_OK) {
    //McuShell_SendStr((unsigned char*)("MATRIX_ExecuteRemoteQueueAndWait: failed waiting for idle!\r\n"), io->stdErr);
    return res;
  }
  return res;
}

uint8_t MATRIX_SendToRemoteQueueExecuteAndWait(const McuShell_StdIOType *io) {
  uint8_t res;

  res = MATRIX_SendToRemoteQueue();
  if (res!=ERR_OK) {
    return res;
  }
  return MATRIX_ExecuteRemoteQueueAndWait(io);
}

uint8_t MATRIX_DrawClockDelays(uint8_t x, uint8_t y, uint8_t delay0, uint8_t delay1) {
    if (x>=MATRIX_NOF_CLOCKS_X || y>=MATRIX_NOF_CLOCKS_Y) {
      return ERR_FRAMING;
    }
    matrix.delayMap[x][y][0] = delay0;
    matrix.delayMap[x][y][1] = delay1;
    return ERR_OK;
}

uint8_t MATRIX_ShowTime(uint8_t hour, uint8_t minute) {
  DrawNumber(&clockDigits[hour/10], 0, 0);
  DrawNumber(&clockDigits[hour%10], 2, 0);
  DrawNumber(&clockDigits[minute/10], 4, 0);
  DrawNumber(&clockDigits[minute%10], 6, 0);
  return MATRIX_SendToRemoteQueueExecuteAndWait(NULL);
}

uint8_t MATRIX_ShowDigit(uint8_t digit) {
  DrawNumber(&clockDigits[digit%10], 0, 0);
  return MATRIX_SendToRemoteQueueExecuteAndWait(NULL);
}

uint8_t MATRIX_ShowTemperature(uint8_t temperature) {
  /* number */
  DrawNumber(&clockDigits[temperature/10], 0, 0);
  DrawNumber(&clockDigits[temperature%10], 2, 0);
  /* degree */
  (void)MATRIX_DrawClockHands(4, 0, 180,  90);
  (void)MATRIX_DrawClockHands(5, 0, 270, 180);
  (void)MATRIX_DrawClockHands(4, 1,   0,  90);
  (void)MATRIX_DrawClockHands(5, 1, 270,   0);
  (void)MATRIX_DrawClockHands(4, 2, 225, 225);
  (void)MATRIX_DrawClockHands(5, 2, 225, 225);
  /* C */
  (void)MATRIX_DrawClockHands(6, 0,  90, 180);
  (void)MATRIX_DrawClockHands(7, 0, 270, 270);
  (void)MATRIX_DrawClockHands(6, 1,   0, 180);
  (void)MATRIX_DrawClockHands(7, 1, 225, 225);
  (void)MATRIX_DrawClockHands(6, 2,   0,  90);
  (void)MATRIX_DrawClockHands(7, 2, 270, 270);
  return MATRIX_SendToRemoteQueueExecuteAndWait(NULL);
}

void MATRIX_WriteNumber(const McuShell_StdIOType *io) {
  MATRIX_MoveAllto12(10000, io);
  DrawNumber(&clockDigits[0], 0, 0);
  DrawNumber(&clockDigits[1], 2, 0);
  DrawNumber(&clockDigits[2], 4, 0);
  DrawNumber(&clockDigits[3], 6, 0);
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(io);
  MATRIX_Delay(4000);
  MATRIX_MoveAllto12(10000, io);

  DrawNumber(&clockDigits[4], 0, 0);
  DrawNumber(&clockDigits[5], 2, 0);
  DrawNumber(&clockDigits[6], 4, 0);
  DrawNumber(&clockDigits[7], 6, 0);
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(io);
  MATRIX_Delay(4000);
  MATRIX_MoveAllto12(10000, io);

  DrawNumber(&clockDigits[8], 0, 0);
  DrawNumber(&clockDigits[9], 2, 0);
  DrawNumber(&clockDigits[8], 4, 0);
  DrawNumber(&clockDigits[9], 6, 0);
  (void)MATRIX_SendToRemoteQueueExecuteAndWait(io);
  MATRIX_Delay(4000);
  MATRIX_MoveAllto12(10000, io);
#if 0
  for(int i=0; i<=9; i++) {
    DrawNumber(&clockDigits[i], 0, 0);
    (void)MATRIX_Update();
    MATRIX_Delay(4000);
  }
  MATRIX_MoveAllto12(10000, io);
#endif
#if 0
  for(int i=0; i<=9; i++) {
    DrawNumber(&clockDigits[i], 2, 0);
    (void)MATRIX_Update();
   MATRIX_Delay(4000);
  }
#endif
}

uint8_t MATRIX_MoveAllto12(int32_t timeoutMs, const McuShell_StdIOType *io) {
  uint8_t res;

  res = MATRIX_DrawAllClockHands(0, 0);
  if (res!=ERR_OK) {
    McuShell_SendStr((unsigned char*)("MoveAllto12: failed drawing hands!\r\n"), io->stdErr);
    return res;
  }
  res = MATRIX_DrawAllClockDelays(8, 8);
  if (res!=ERR_OK) {
    McuShell_SendStr((unsigned char*)("MoveAllto12: failed setting delays!\r\n"), io->stdErr);
    return res;
  }
  res = MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  if (res!=ERR_OK) {
    McuShell_SendStr((unsigned char*)("MoveAllto12: failed setting mode!\r\n"), io->stdErr);
    return res;
  }
  return MATRIX_SendToRemoteQueueExecuteAndWait(io);
}

uint8_t MATRIX_DrawAllClockDelays(uint8_t delay0, uint8_t delay1) {
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      matrix.delayMap[x][y][0] = delay0;
      matrix.delayMap[x][y][1] = delay1;
    }
  }
  return ERR_OK;
}

#endif /* PL_CONFIG_IS_MASTER */
