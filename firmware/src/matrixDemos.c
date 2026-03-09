#include "platform.h"
#if PL_CONFIG_IS_MASTER
#include "matrixconfig.h"
#include "matrixDemos.h"
#include "matrixMaster.h"

static uint8_t MATRIX_FailedDemo(uint8_t res) {
  return res; /* used to set a breakpoint in case of failure */
}

uint8_t MATRIX_Test0(const McuShell_StdIOType *io) {
  MATRIX_DrawAllClockDelays(0, 0);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(180, 0);
  MATRIX_SendToRemoteQueue();
  MATRIX_DrawAllClockHands(0, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(io);
  MATRIX_MoveAllto12(10000, io);
  return ERR_OK;
}

uint8_t MATRIX_Demo(const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;

  MATRIX_MoveAllto12(10000, io);

  MATRIX_ShowDigit(2);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_Demo4(io);
  res = MATRIX_WaitForIdle(30000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_ShowDigit(3);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_Demo3(io);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_ShowDigit(5);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_Demo5(io);
  res = MATRIX_WaitForIdle(30000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_ShowDigit(6);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_Delay(3000);

  MATRIX_MoveAllto12(10000, io);

  MATRIX_Demo2(io);
  res = MATRIX_WaitForIdle(30000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }


  MATRIX_ShowDigit(7);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_Demo3(io);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  /*
  MATRIX_ShowTime(20, 34);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  MATRIX_Delay(3000);

  MATRIX_Demo3(io);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_ShowTemperature(22);
  MATRIX_Delay(3000);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_Demo2(io);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_ShowTime(20, 35);
  res = MATRIX_WaitForIdle(15000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  MATRIX_Delay(3000);
  */

  MATRIX_MoveAllto12(10000, io);
  return res;
}

uint8_t MATRIX_Demo0(const McuShell_StdIOType *io) {
  uint8_t res;

  /* set move mode: */
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      MATRIX_DrawClockDelays(x, y, 8+y, 8+y);
    }
  }
  /* set movement: */
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 90, 270);
    }
  }
  res = MATRIX_SendToRemoteQueue(); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0, 180);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(io); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  /* move to park position */
  res = MATRIX_MoveAllto12(20000, io);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  return res;
}

uint8_t MATRIX_Demo1(const McuShell_StdIOType *io) {
  int angle0, angle1;
  uint8_t res;

  /* init at 12 */
  res = MATRIX_MoveAllto12(10000, io);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  /* move all clocks to '|' position */
  angle0 = 0;
  angle1 = 180;
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, angle0, angle1);
    }
  }
  (void)MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);

  (void)MATRIX_DrawAllClockDelays(6, 6);
  res = MATRIX_SendToRemoteQueue(); /* queue commands */
  if (res!=ERR_OK) {
    McuShell_SendStr((unsigned char*)"Failed Demo1: Point 1\r\n", io->stdErr);
    return MATRIX_FailedDemo(res);
  }
  /* rotate them to the next quadrant */
  (void)MATRIX_DrawAllClockDelays(6, 12);
  for(int i=0; i<3; i++) {
    angle0 = (angle0+90)%360;
    angle1 = (angle1+90)%360;
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
        (void)MATRIX_DrawClockHands(x, y, angle0, angle1);
      }
    }
    res = MATRIX_SendToRemoteQueue(); /* queue command */
    if (res!=ERR_OK) {
      McuShell_SendStr((unsigned char*)"Failed Demo1: Point 2\r\n", io->stdErr);
      return MATRIX_FailedDemo(res);
    }
  }
  /* execute */
  res = MATRIX_ExecuteRemoteQueueAndWait(io);
  if (res!=ERR_OK) {
   McuShell_SendStr((unsigned char*)"Failed Demo1: Point 5\r\n", io->stdErr);
   return MATRIX_FailedDemo(res);
  }
  /* move to park position */
  res = MATRIX_MoveAllto12(20000, io);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  return res;
}

uint8_t MATRIX_Demo2(const McuShell_StdIOType *io) {
  int angle0, angle1;
  uint8_t res;

  (void)MATRIX_DrawAllClockHands(0, 180);
  res = MATRIX_SendToRemoteQueue(); /* queue command */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  /* configure delays */
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockDelays(x, y, 5+x, 5+x);
    }
  }

  /* rotate them to the next quadrant */
  angle0 = angle1 = 0;
  for(int i=0; i<3; i++) {
    angle0 = (angle0+90)%360;
    angle1 = (angle1+90)%360;
    (void)MATRIX_DrawAllClockHands(angle0, angle1);
    res = MATRIX_SendToRemoteQueue(); /* queue command */
    if (res!=ERR_OK) {
      return MATRIX_FailedDemo(res);
    }
  }
  /* execute */
  res = MATRIX_ExecuteRemoteQueueAndWait(io);
  return res;
}

uint8_t MATRIX_Demo3(const McuShell_StdIOType *io) {
  uint8_t res;

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawMoveMode(x, y, STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
      MATRIX_DrawClockDelays(x, y, 8, 8);
    }
  }
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0, 180);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(io); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawMoveMode(x, y, STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
      MATRIX_DrawClockDelays(x, y, 5+x, 5+x);
    }
  }
  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 180, 0);
    }
  }
  res = MATRIX_SendToRemoteQueue();
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }


  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      (void)MATRIX_DrawClockHands(x, y, 0, 180);
    }
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(io);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  return ERR_OK;
}

uint8_t MATRIX_Demo4(const McuShell_StdIOType *io) {
  uint8_t res;

  MATRIX_MoveAllto12(10000, io);

  MATRIX_DrawAllClockDelays(8, 8);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(180, 180);
  res = MATRIX_SendToRemoteQueue(); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_DrawAllClockHands(0, 0);
  res = MATRIX_SendToRemoteQueue(); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(0, 90);
  res = MATRIX_SendToRemoteQueue(); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  MATRIX_DrawAllClockHands(270, 0);
  res = MATRIX_SendToRemoteQueueExecuteAndWait(io); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  /* move to park position */
  res = MATRIX_WaitForIdle(20000);
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  return MATRIX_MoveAllto12(10000, io);
}

uint8_t MATRIX_Demo5(const McuShell_StdIOType *io) {
  uint8_t res;

  MATRIX_DrawAllClockDelays(10, 10);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  for(int x=0; x<MATRIX_NOF_CLOCKS_X; x+=2) {
    MATRIX_DrawClockHands(x, 0, 135, 135);
    MATRIX_DrawClockHands(x, 1,  45,  45);
    MATRIX_DrawClockHands(x, 2, 135, 135);

    MATRIX_DrawClockHands(x+1, 0, 225, 225);
    MATRIX_DrawClockHands(x+1, 1, 315, 315);
    MATRIX_DrawClockHands(x+1, 2, 225, 225);
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(io); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CCW);
  for(int x=0; x<MATRIX_NOF_CLOCKS_X; x+=2) {
    MATRIX_DrawClockHands(x, 0, 135-1, 135+1);
    MATRIX_DrawClockHands(x, 1,  45-1,  45+1);
    MATRIX_DrawClockHands(x, 2, 135-1, 135+1);

    MATRIX_DrawClockHands(x+1, 0, 225-1, 225+1);
    MATRIX_DrawClockHands(x+1, 1, 315-1, 315+1);
    MATRIX_DrawClockHands(x+1, 2, 225-1, 225+1);
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(io); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CW);
  for(int x=0; x<MATRIX_NOF_CLOCKS_X; x+=2) {
    MATRIX_DrawClockHands(x, 0, 135, 135);
    MATRIX_DrawClockHands(x, 1,  45,  45);
    MATRIX_DrawClockHands(x, 2, 135, 135);

    MATRIX_DrawClockHands(x+1, 0, 225, 225);
    MATRIX_DrawClockHands(x+1, 1, 315, 315);
    MATRIX_DrawClockHands(x+1, 2, 225, 225);
  }
  res = MATRIX_SendToRemoteQueueExecuteAndWait(io); /* queue commands */
  if (res!=ERR_OK) {
    return MATRIX_FailedDemo(res);
  }
  return ERR_OK;
}

uint8_t MATRIX_Demo6(const McuShell_StdIOType *io) {
  MATRIX_DrawAllClockDelays(1, 1);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  for(int i=2;i<12;i++) {
    MATRIX_ShowTime(21, i);
    (void)MATRIX_WaitForIdle(10000);
    MATRIX_Delay(800);
  }

  MATRIX_MoveAllto12(10000, io);
  return ERR_OK;
}

uint8_t MATRIX_Demo7(const McuShell_StdIOType *io) {
  MATRIX_DrawAllClockDelays(10, 10);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(270, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(io);
  MATRIX_MoveAllto12(10000, io);
  return ERR_OK;
}

static uint8_t Intermezzo0(void) {
  MATRIX_DrawAllClockDelays(40, 40);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(0, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(NULL);

  MATRIX_DrawAllClockDelays(30, 30);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CCW, STEPPER_MOVE_MODE_CCW);
  MATRIX_DrawAllClockHands(90, 270);
  MATRIX_SendToRemoteQueueExecuteAndWait(NULL);
  return ERR_OK;
}

static uint8_t Intermezzo1(void) {
  MATRIX_DrawAllClockDelays(40, 40);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(0, 90);
  MATRIX_SendToRemoteQueueExecuteAndWait(NULL);

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(315, 45);
  MATRIX_SendToRemoteQueueExecuteAndWait(NULL);
  return ERR_OK;
}

static uint8_t Intermezzo2(void) {
  MATRIX_DrawAllClockDelays(40, 40);
  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  MATRIX_DrawAllClockHands(0, 180);
  MATRIX_SendToRemoteQueueExecuteAndWait(NULL);

  MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_CW, STEPPER_MOVE_MODE_CW);
  MATRIX_DrawAllClockHands(180, 0);
  MATRIX_SendToRemoteQueueExecuteAndWait(NULL);
  return ERR_OK;
}

uint8_t MATRIX_Intermezzo(uint8_t *nr) {
  /* iterate through the list of intermezzos */
  switch(*nr) {
    case 0:
      (*nr)++;
      return Intermezzo0();
    case 1:
      (*nr)++;
      return Intermezzo1();
    case 2:
      (*nr) = 0;
      return Intermezzo2();
    default:
      break;
  }
  return ERR_FAILED;
}

#endif /* PL_CONFIG_IS_MASTER */
