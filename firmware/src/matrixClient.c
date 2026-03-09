#include "platform.h"
#if PL_CONFIG_IS_CLIENT
#include "matrixconfig.h"
#include "matrixClient.h"
#include "stepper.h"
#include "rs485.h"
#include "McuUtility.h"
#include "McuWait.h"
#include "Shell.h"
#if PL_CONFIG_USE_MAG_SENSOR
  #include "magnets.h"
#endif
#include "nvmc.h"
#include "StepperBoard.h"

int const mapXYBoardPosNr[][MATRIX_NOF_CLOCKS_BOARD_Y] = {{1, 2, 4}, {0, 3, 5}};

int MATRIX_GetClockPosition(int32_t x, int32_t y) {
	return mapXYBoardPosNr[x][y];
}

void MATRIX_SetZeroPosition(STEPPER_Handle_t *motors[], size_t nofMotors) {
  /* set zero position */
  for(int i=0; i<nofMotors; i++) {
    STEPPER_SetPos(motors[i], 0);
  }
}

STEPPER_Handle_t MATRIX_GetStepper(int32_t x, int32_t y, int32_t z) {
  STEPPER_Handle_t stepper;
  STEPBOARD_Handle_t board;

  if (x>=MATRIX_NOF_CLOCKS_X || y>=MATRIX_NOF_CLOCKS_Y || z>=MATRIX_NOF_CLOCKS_Z) {
    return NULL;
  }
  board = STEPBOARD_GetBoard();
  if (board==NULL) {
    return NULL;
  }
  stepper = STEPBOARD_GetStepper(board, MATRIX_GetClockPosition(x, y), z);
  return stepper;
}

void MATRIX_MoveByOffset(STEPPER_Handle_t *motors[], int16_t offsets[], size_t nofMotors, uint16_t delay) {
  /* here all hands are on the sensor: adjust with offset */
   for(int i=0; i<nofMotors; i++) {
     STEPPER_MoveMotorStepsRel(motors[i], offsets[i], delay);
   } /* for */
   STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
}

uint8_t MATRIX_STEPPER_MoveHandOnSensor(STEPPER_Handle_t stepper, bool onSensor, int32_t stepSize, int32_t timeoutms, uint32_t waitms, uint16_t delay) {
  X12_Stepper_t *s = STEPPER_GetDevice(stepper);

  while (MAG_IsTriggered(s->mag)!=onSensor && timeoutms >= 0) {
  	STEPPER_MoveMotorStepsRel(stepper, stepSize, delay); /* make by 1 degree */
  	STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), waitms);
  	timeoutms -= waitms;
  }

  if (timeoutms < 0) {
    return ERR_UNDERFLOW;
  }
  return ERR_OK;
}

uint8_t MATRIX_STEPPER_ZeroHand(STEPPER_Handle_t stepper, int16_t offset, uint16_t delay) {
	uint8_t res = ERR_OK;
	X12_Stepper_t *s;

	s = STEPPER_GetDevice(stepper);
	if (MAG_IsTriggered(s->mag)) { /* hand on sensor? */
	  STEPPER_MoveMotorDegreeRel(stepper, 90, delay); /* move away from sensor */
	  STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
	}

	/* move forward in larger steps to find sensor */
	if (MATRIX_STEPPER_MoveHandOnSensor(stepper, true, 20, 10000, 10, delay)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	/* step back in micro-steps just to leave the sensor */
	if (MATRIX_STEPPER_MoveHandOnSensor(stepper, false, -1, 10000, 10, delay)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	/* step forward in micro-steps just to enter the sensor again */
	if (MATRIX_STEPPER_MoveHandOnSensor(stepper, true, 1, 10000, 2, delay)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	/* here all hands are on the sensor: adjust with offset */
	STEPPER_MoveMotorStepsRel(stepper, offset, delay);
	STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
	STEPPER_SetPos(stepper, 0);
	return res;
}

uint8_t MATRIX_MoveHandOnSensor(STEPPER_Handle_t *motors[], size_t nofMotors, bool onSensor, int32_t stepSize, int32_t timeoutms, uint32_t waitms, uint16_t delay) {
  uint8_t res = ERR_OK;
  bool done;
  X12_Stepper_t *s;

  /* move hand over sensor */
  for(;;) { /* breaks */
    done = true;
    for(int i=0; i<nofMotors; i++) { /* check if all motors are on sensor */
      s = STEPPER_GetDevice(motors[i]);
      if (MAG_IsTriggered(s->mag)!=onSensor) {
        done = false; /* not yet */
        break;
      }
    }
    if (done || timeoutms<0) { /* all hands on sensor */
      break;
    }
    for(int i=0; i<nofMotors; i++) {
      s = STEPPER_GetDevice(motors[i]);
      if (MAG_IsTriggered(s->mag)!=onSensor) {
        STEPPER_MoveMotorStepsRel(motors[i], stepSize, delay); /* make by 1 degree */
      }
    } /* for */
    STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), waitms);
    timeoutms -= waitms;
  }
  if (timeoutms<0) {
    res = ERR_UNDERFLOW;
  }
  return res;
}

uint8_t MATRIX_ZeroHand(STEPPER_Handle_t *motors[], int16_t offsets[], size_t nofMotors, uint16_t delay) {
  uint8_t res = ERR_OK;
  X12_Stepper_t *s;

  /* if hand is on sensor: move hand out of the sensor area */
  for(int i=0; i<nofMotors; i++) {
    s = STEPPER_GetDevice(motors[i]);
    if (MAG_IsTriggered(s->mag)) { /* hand on sensor? */
      STEPPER_MoveMotorDegreeRel(motors[i], 90, delay); /* move away from sensor */
    }
  } /* for */
  STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);

  /* move forward in larger steps to find sensor */
  if (MATRIX_MoveHandOnSensor(motors, nofMotors, true, 20, 10000, 10, delay)!=ERR_OK) {
    res = ERR_FAILED;
  }

  /* step back in micro-steps just to leave the sensor */
  if (MATRIX_MoveHandOnSensor(motors, nofMotors, false, -1, 10000, 10, delay)!=ERR_OK) {
    res = ERR_FAILED;
  }

  /* step forward in micro-steps just to enter the sensor again */
  if (MATRIX_MoveHandOnSensor(motors, nofMotors, true, 1, 10000, 2, delay)!=ERR_OK) {
    res = ERR_FAILED;
  }

  /* here all hands are on the sensor: adjust with offset */
  MATRIX_MoveByOffset(motors, offsets, nofMotors, delay);
  /* set zero position */
  MATRIX_SetZeroPosition(motors, nofMotors);
  return res;
}

uint8_t MATRIX_AllHandsOff(int32_t z) {
	uint8_t res = ERR_OK;
	STEPPER_Handle_t *motors[STEPBOARD_NOF_CLOCKS];
	int16_t offsets[STEPBOARD_NOF_CLOCKS];
	int i;

	/* fill in motor array information */
	i = 0;
	for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
	  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
	  	motors[i] = MATRIX_GetStepper(x, y, z);
	  	offsets[i] = 1080;
	    i++;
	  }
	}

	if (MATRIX_MoveHandOnSensor(motors, STEPBOARD_NOF_CLOCKS, false, 10, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	MATRIX_MoveByOffset(motors, offsets, STEPBOARD_NOF_CLOCKS, STEPPER_HAND_ZERO_DELAY);

	return res;
}

uint8_t MATRIX_ZeroAllHands(void) {
  uint8_t res = ERR_OK;
  STEPPER_Handle_t *motors[STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS];
  int16_t offsets[STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS];
  int i;

  /* fill in motor array information */
  i = 0;
  for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
        motors[i] = MATRIX_GetStepper(x, y, z);
        offsets[i] = NVMC_GetStepperZeroOffset(MATRIX_GetClockPosition(x, y), z);
        i++;
      }
    }
  }
  /* zero all of them */
  if (MATRIX_ZeroHand(motors, offsets, STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
    res = ERR_FAILED;
  }
  return res;
}

uint8_t MATRIX_MoveAllto(int32_t timeoutMs, int32_t degree) {
  int x, y, z;

  for(x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
   for(y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
     for(z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
       STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, z), degree, STEPPER_MOVE_MODE_SHORT, 2, true, true);
     }
   }
 }
 STEPPER_StartTimer();
 return ERR_OK;
}

uint8_t MATRIX_ZeroClock(int32_t x, int32_t y, const McuShell_StdIOType *io) {
	uint8_t res = ERR_OK;
	STEPPER_Handle_t stepper;
	X12_Stepper_t *s;
	const NVMC_Data_t *data;

	data = NVMC_GetDataPtr();

	stepper = MATRIX_GetStepper(x, y, 0);

	s = STEPPER_GetDevice(stepper);
	McuX12_017_ResetDriver(s->x12device);

  res = MATRIX_STEPPER_ZeroHand(stepper, data->zeroOffsets[MATRIX_GetClockPosition(x, y)][0], STEPPER_HAND_ZERO_DELAY);
  if (res!=ERR_OK) {
    McuShell_SendStr((unsigned char*)"failed to find magnet/zero position\r\n", io->stdErr);
    return res;
  }

  STEPPER_MoveMotorStepsRel(stepper, -1080, STEPPER_HAND_ZERO_DELAY);
  STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 1000);

  stepper = MATRIX_GetStepper(x, y, 1);
	res = MATRIX_STEPPER_ZeroHand(stepper, data->zeroOffsets[MATRIX_GetClockPosition(x, y)][1], STEPPER_HAND_ZERO_DELAY);
	if (res!=ERR_OK) {
	  McuShell_SendStr((unsigned char*)"failed to find magnet/zero position\r\n", io->stdErr);
	  return res;
	}

	STEPPER_MoveMotorStepsRel(stepper, -1080, STEPPER_HAND_ZERO_DELAY);
	STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 1000);

	return res;
}

static uint8_t MATRIX_ZeroStepperByMid(STEPPER_Handle_t st) {
	int32_t leftPos = 0;
	int32_t rightPos = 400;
	int32_t offset = 0;
	int32_t stepSize = 1;
	bool onRight = false;

	/* move forward in larger steps to find sensor */
	if (MATRIX_STEPPER_MoveHandOnSensor(st, true, 10, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  return ERR_FAILED;
	}

	/* step back in micro-steps just to leave the sensor */
  if (MATRIX_STEPPER_MoveHandOnSensor(st, false, -1, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
  	return ERR_FAILED;
  }

  /* step forward in micro-steps just to enter the sensor again */
  if (MATRIX_STEPPER_MoveHandOnSensor(st, true, 1, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
  	return ERR_FAILED;
  }

  STEPPER_SetPos(st, 0);

	while(abs(rightPos - leftPos) > 10)
	{
    if (MATRIX_STEPPER_MoveHandOnSensor(st, false, stepSize, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
    	return ERR_FAILED;
    }

    stepSize = -stepSize;

    if (MATRIX_STEPPER_MoveHandOnSensor(st, true, stepSize, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
    	return ERR_FAILED;
    }

    onRight = !onRight;

    if (onRight) {
    	rightPos = abs(STEPPER_GetPos(st));
    } else {
    	leftPos = abs(STEPPER_CLOCK_360_STEPS - STEPPER_GetPos(st));
    }

    STEPPER_SetPos(st, 0);

	}

  offset = (rightPos + leftPos) / 4;

  if (onRight) {
	  offset = -offset;
  }

  /* here all hands are on the sensor: adjust with offset */
  STEPPER_MoveMotorStepsRel(st, offset, STEPPER_HAND_ZERO_DELAY);
  STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
  STEPPER_SetPos(st, 0);

  return ERR_OK;
}

uint8_t MATRIX_ZeroMatrixByMid() {
	uint8_t res = ERR_OK;
	STEPPER_Handle_t *motors[STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS];

	/* all hands off */
	res = MATRIX_AllHandsOff(0);
	if (res!=ERR_OK) {
		return res;
	}
	res = MATRIX_AllHandsOff(1);
	if (res!=ERR_OK) {
		return res;
	}


	X12_Stepper_t *xs = STEPPER_GetDevice(MATRIX_GetStepper(0, 0, 0));
	McuX12_017_ResetDriver(xs->x12device);


	int i = 0;
	for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
	  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
	    for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
	    	motors[i] = MATRIX_GetStepper(x, y, z);

	    	MATRIX_ZeroStepperByMid(motors[i]);

	      STEPPER_MoveMotorStepsRel(motors[i], -1080, STEPPER_HAND_ZERO_DELAY);
	      STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);

	      i++;
	    }
	  }
	}

	if (res == ERR_OK) {
	   for(int j=0; j<i; j++) {
	     STEPPER_MoveMotorStepsRel(motors[j], 1080, STEPPER_HAND_ZERO_DELAY);
	   } /* for */
	   STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
	}

	return res;
}


uint8_t MATRIX_ZeroBoardHands() {
	uint8_t res = ERR_OK;
	STEPPER_Handle_t *motors[STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS];
	const NVMC_Data_t *offsets;


	/* all hands off */
	res = MATRIX_AllHandsOff(0);
	if (res!=ERR_OK) {
		return res;
	}
	res = MATRIX_AllHandsOff(1);
	if (res!=ERR_OK) {
		return res;
	}


	X12_Stepper_t *xs = STEPPER_GetDevice(MATRIX_GetStepper(0, 0, 0));
	McuX12_017_ResetDriver(xs->x12device);

	offsets = NVMC_GetDataPtr();


	int i = 0;
	for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
	  for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
	    for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
	    	motors[i] = MATRIX_GetStepper(x, y, z);

	      /* move forward in larger steps to find sensor */
	      if (MATRIX_STEPPER_MoveHandOnSensor(motors[i], true, 10, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	        res = ERR_FAILED;
	      }

	      /* step back in micro-steps just to leave the sensor */
	      if (MATRIX_STEPPER_MoveHandOnSensor(motors[i], false, -1, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	        res = ERR_FAILED;
	      }

	      /* step forward in micro-steps just to enter the sensor again */
	      if (MATRIX_STEPPER_MoveHandOnSensor(motors[i], true, 1, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	        res = ERR_FAILED;
	      }

	      STEPPER_MoveMotorStepsRel(motors[i], offsets->zeroOffsets[MATRIX_GetClockPosition(x, y)][z], STEPPER_HAND_ZERO_DELAY);
	      STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);

	      STEPPER_SetPos(motors[i], 0);

	      STEPPER_MoveMotorStepsRel(motors[i], -1080, STEPPER_HAND_ZERO_DELAY);
	      STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);

	      i++;
	    }
	  }
	}

	if (res == ERR_OK) {
	   for(int j=0; j<i; j++) {
	     STEPPER_MoveMotorStepsRel(motors[j], 1080, STEPPER_HAND_ZERO_DELAY);
	   } /* for */
	   STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
	}

	return res;
}

uint8_t MATRIX_SetOffsetSteppersFrom12(int32_t x, int32_t y, int32_t z, const McuShell_StdIOType *io) {
	uint8_t res = ERR_OK;
	STEPPER_Handle_t stepper;

	stepper = MATRIX_GetStepper(x, y, z);
	STEPPER_SetPos(stepper, 0);

	if (MATRIX_STEPPER_MoveHandOnSensor(stepper, true, -10, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	/* step back in micro-steps just to leave the sensor */
	if (MATRIX_STEPPER_MoveHandOnSensor(stepper, false, 1, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	/* step forward in micro-steps just to enter the sensor again */
	if (MATRIX_STEPPER_MoveHandOnSensor(stepper, true, -1, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  res = ERR_FAILED;
	  return res;
	}

	uint8_t buf[50];

	McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"\r\n ");
	McuUtility_strcatNum32u(buf, sizeof(buf), STEPPER_GetPos(stepper));
	McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
	McuShell_SendStatusStr((unsigned char*)"Offset", buf, io->stdOut);

	/* store new offsets */
	/*
	offset = -STEPPER_GetPos(stepper);
	MATRIX_SetOffset(x, y, z, offset);

	STEPPER_MoveMotorStepsRel(stepper, offset, STEPPER_HAND_ZERO_DELAY);
	STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
	*/

	return res;
}

uint8_t MATRIX_SetBacklashSteps(int32_t x, int32_t y, int32_t z, int16_t b) {
	uint8_t res = ERR_OK;

	NVMC_Data_t data;

  data = *NVMC_GetDataPtr(); /* struct copy */
	data.backlashsteps[MATRIX_GetClockPosition(x, y)][z] = b;

	res = NVMC_WriteConfig(&data);
	if (res!=ERR_OK) {
	  return res;
	}

	return res;
}

uint8_t MATRIX_SetOffset(int32_t x, int32_t y, int32_t z, int16_t b) {
	uint8_t res = ERR_OK;

	NVMC_Data_t data;

  data = *NVMC_GetDataPtr(); /* struct copy */
	data.zeroOffsets[MATRIX_GetClockPosition(x, y)][z] = b;

	res = NVMC_WriteConfig(&data);
	if (res!=ERR_OK) {
	  return res;
	}

	return res;
}

int16_t MATRIX_GetOffset(int32_t x, int32_t y, int32_t z) {
	NVMC_Data_t data;

  data = *NVMC_GetDataPtr();

	return data.zeroOffsets[MATRIX_GetClockPosition(x, y)][z];
}

uint8_t MATRIX_SetStepper12(int32_t x, int32_t y, int32_t z, int32_t offset) {
	uint8_t res = ERR_OK;
	STEPPER_Handle_t *motors[1];
	STEPPER_Handle_t stepper;
	//int32_t offset;

	//if (MATRIX_AllHandsOff(z)) {
	//	res = ERR_FAILED;
	//}

	stepper = MATRIX_GetStepper(x, y, z);

	X12_Stepper_t *xs = STEPPER_GetDevice(stepper);
	McuX12_017_ResetDriver(xs->x12device);

	motors[0] = stepper;

	if (MATRIX_MoveHandOnSensor(motors, 1, true, -10, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	/* step back in micro-steps just to leave the sensor */
	if (MATRIX_MoveHandOnSensor(motors, 1, false, 1, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  res = ERR_FAILED;
	}

	/* step forward in micro-steps just to enter the sensor again */
	if (MATRIX_MoveHandOnSensor(motors, 1, true, -1, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
	  res = ERR_FAILED;
	  return res;
	}

	if (offset < -1080 || offset > 1080) {
		offset = MATRIX_GetOffset(x, y, z);
	}

	STEPPER_MoveMotorStepsRel(stepper, offset, STEPPER_HAND_ZERO_DELAY);
	STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);

	return res;
}

uint8_t MATRIX_SetOffsetFrom12(void) {
  /* all hands shall be at 12-o-clock position */
  uint8_t res = ERR_OK;
  STEPPER_Handle_t *motors[STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS];
  int16_t offsets[STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS];
  STEPPER_Handle_t stepper;
  int i;

  /* first zero position at current position and set delay */
  i = 0;
  for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
        stepper = MATRIX_GetStepper(x, y, z);
        STEPPER_SetPos(stepper, 0);
        motors[i] = stepper;
        i++;
      }
    }
  }

  /* move forward in larger steps to find sensor */
  if (MATRIX_MoveHandOnSensor(motors, sizeof(motors)/sizeof(motors[0]), true, -10, 10000, 5, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
    res = ERR_FAILED;
  }

  /* step back in micro-steps just to leave the sensor */
  if (MATRIX_MoveHandOnSensor(motors, sizeof(motors)/sizeof(motors[0]), false, 1, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
    res = ERR_FAILED;
  }

  /* step forward in micro-steps just to enter the sensor again */
  if (MATRIX_MoveHandOnSensor(motors, sizeof(motors)/sizeof(motors[0]), true, -1, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
    res = ERR_FAILED;
    return res;
  }

  /* store new offsets */
  NVMC_Data_t data;

  data = *NVMC_GetDataPtr(); /* struct copy */
  for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
        stepper = MATRIX_GetStepper(x, y, z);
        data.zeroOffsets[MATRIX_GetClockPosition(x, y)][z] = -STEPPER_GetPos(stepper);
      }
    }
  }
  res = NVMC_WriteConfig(&data);
  if (res!=ERR_OK) {
    return res;
  }
  /* fill in motor array information */
  for(int c=0; c<STEPBOARD_NOF_CLOCKS; c++) {
    for (int m=0; m<STEPPER_NOF_CLOCK_MOTORS; m++) {
      offsets[c*STEPPER_NOF_CLOCK_MOTORS + m] = NVMC_GetStepperZeroOffset(c, m);
    }
  }
  MATRIX_MoveByOffset(motors, offsets, sizeof(motors)/sizeof(motors[0]), STEPPER_HAND_ZERO_DELAY);
  return res;
}

uint8_t MATRIX_MoveAllto12(int32_t timeoutMs, const McuShell_StdIOType *io) {
  return MATRIX_MoveAllto(timeoutMs, 0);
}

int32_t X12_GetBacklashSteps(void *dev, Coord_t coord, int steps) {
  X12_Stepper_t *device = (X12_Stepper_t*)dev;
  bool isForward;
  int32_t backlashSteps;

  isForward = McuX12_017_IsForward(device->x12device, device->x12motor);
  backlashSteps = NVMC_GetStepperBacklashSteps(coord.clock, coord.motor);

  if (steps<0 && isForward) {
  	return -backlashSteps;
  } else if (steps>0 && !isForward) {
  	return backlashSteps;
  } else {
  	return 0;
  }
}

void X12_SingleStep(void *dev, int step) {
  X12_Stepper_t *device = (X12_Stepper_t*)dev;

  McuX12_017_SingleStep(device->x12device, device->x12motor, step);
}

#endif /* PL_CONFIG_IS_CLIENT */
