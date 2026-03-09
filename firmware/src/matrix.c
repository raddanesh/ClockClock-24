#include "platform.h"
#if PL_CONFIG_USE_MATRIX
#include "matrixconfig.h"
#if PL_CONFIG_IS_MASTER
  #include "matrix.h"
#endif
#include "rs485.h"
#include "McuUtility.h"
#include "McuWait.h"
#include "stepper.h"
#include "Shell.h"
#if PL_CONFIG_USE_WDT
  #include "watchdog.h"
#endif
#include "StepperBoard.h"
#if PL_CONFIG_USE_X12_STEPPER
  #include "McuX12_017.h"
#endif
#if PL_CONFIG_USE_MAG_SENSOR
  #include "magnets.h"
#endif
#include "nvmc.h"
#if PL_CONFIG_IS_CLIENT
  #include "matrixClient.h"
#else
  #include "matrixMaster.h"
  #include "matrixDemos.h"
#endif
#include "matrixShell.h"

bool MATRIX_ExecuteQueue = false;

#if PL_CONFIG_USE_STEPPER
static STEPBOARD_Handle_t MATRIX_Boards[MATRIX_NOF_BOARDS];

static X12_Stepper_t x12Steppers[12]; /* the 12 stepper motors on a board */

STEPBOARD_Handle_t MATRIX_AddrGetBoard(uint8_t addr) {
  for(int i=0; i<MATRIX_NOF_BOARDS; i++){
    if (STEPBOARD_GetAddress(MATRIX_Boards[i])==addr) {
      return MATRIX_Boards[i];
    }
  }
  return NULL;
}
#endif

uint8_t MATRIX_GetAddress(int32_t x, int32_t y, int32_t z) {
#if PL_CONFIG_IS_MASTER
  return MATRIX_GetClock(x, y).addr;
#else
  return RS485_GetAddress();
#endif
}

uint8_t MATRIX_GetPos(int32_t x, int32_t y, int32_t z) {
#if PL_CONFIG_IS_MASTER
  return MATRIX_GetClock(x, y).nr;
#else
  return MATRIX_GetClockPosition(x, y);
#endif
}

#if PL_CONFIG_USE_MAG_SENSOR
/*
static uint8_t MATRIX_Test(void) {
  // Test the clock stepper motors. Pass -1 to run the test for all motors/clocks
  for (int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
        STEPPER_MoveClockDegreeRel(MATRIX_GetStepper(x, y, z), 90, STEPPER_MOVE_MODE_CW, 4, true, true);
        STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 1000);
      }
    }
  }
  for (int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
    for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
      for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
        STEPPER_MoveClockDegreeRel(MATRIX_GetStepper(x, y, z), -90, STEPPER_MOVE_MODE_CCW, 4, true, true);
        STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 1000);
      }
    }
  }
  return ERR_OK;
}
*/

#endif

#if PL_CONFIG_USE_STEPPER
void MATRIX_TimerCallback(void) {
  bool workToDo = false;
  STEPPER_Handle_t stepper;

  /* go through all boards and update steps */
  for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
    for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
       for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) { /* go through all motors */
         stepper = MATRIX_GetStepper(x, y, z);
         workToDo |= STEPPER_TimerClockCallback(stepper);
      } /* for */
    }
  }
  /* check if we can stop timer */
  if (!workToDo) {
    STEPPER_StopTimer();
  }
}

static void MatrixQueueTask(void *pv) {
  uint8_t *cmd;
  bool noCommandsInQueue = true;
  McuShell_ConstStdIOType *io = McuShell_GetStdio();
  STEPPER_Handle_t stepper;
  uint8_t command[96];

  for(;;) {
    noCommandsInQueue = true;
    if (MATRIX_ExecuteQueue) {
      for(int x=0; x<MATRIX_NOF_CLOCKS_X; x++) {
        for(int y=0; y<MATRIX_NOF_CLOCKS_Y; y++) {
          for(int z=0; z<MATRIX_NOF_CLOCKS_Z; z++) {
            stepper = MATRIX_GetStepper(x, y, z);
            if (STEPPER_IsIdle(stepper)) { /* no steps to do? */
              if (xQueueReceive(STEPPER_GetQueue(stepper), &cmd, 0)==pdPASS) { /* check queue */
                noCommandsInQueue = false;
                McuUtility_strcpy(command, sizeof(command), (unsigned char*)"matrix ");
                if ((cmd[0]=='a' || cmd[0]=='r') && cmd[1] == ' ') {
                  McuUtility_chcat(command, sizeof(command), cmd[0]);
                  McuUtility_chcat(command, sizeof(command), ' ');
                  McuUtility_strcatNum8u(command, sizeof(command), x);
                  McuUtility_chcat(command, sizeof(command), ' ');
                  McuUtility_strcatNum8u(command, sizeof(command), y);
                  McuUtility_chcat(command, sizeof(command), ' ');
                  McuUtility_strcatNum8u(command, sizeof(command), z);
                  McuUtility_chcat(command, sizeof(command), ' ');
                  McuUtility_strcat(command, sizeof(command), cmd+2);
                } else { /* error? */
                  McuUtility_strcat(command, sizeof(command), cmd);
                }
                //McuShell_SendStr(command, io->stdOut);
                //McuShell_SendStr((unsigned char*)"\r\n", io->stdOut);
                bool handled = false;
                if (MATRIX_ParseCommand(command, &handled, io) !=ERR_OK || !handled) { /* parse and execute it */
                  McuShell_SendStr((unsigned char*)"Failed executing queued command!\r\n", io->stdErr);
                }
              //  if (SHELL_ParseCommand(command, io, true)!=ERR_OK) { /* parse and execute it */
              //    McuShell_SendStr((unsigned char*)"Failed executing queued command!\r\n", io->stdErr);
              //  }
                vPortFree(cmd); /* free memory for command */
              }
            }
          } /* all motors on clock */
        } /* all clocks on board */
      } /* for all boards */
      if (noCommandsInQueue) { /* nothing pending in queues */
        bool allIdle = true;
        for(int b=0; b<MATRIX_NOF_BOARDS; b++) {
          if (STEPBOARD_IsIdle(MATRIX_Boards[b])) {
            //STEPBOARD_NormalizePosition(MATRIX_Boards[b]); /* \todo could get a race condition here! */
            /* system seems idle but is not:
             * matrix R 0 4 0 360 0 CW
             * matrix r 0 4 0 360 0 CC
             */
          } else {
            allIdle = false;
          }
        }
        if (allIdle) {
          MATRIX_ExecuteQueue = false;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(5));
  } /* for */
}
#endif

#if PL_CONFIG_USE_X12_STEPPER
static void InitSteppers(void) {
  McuX12_017_Config_t x12config;
  McuX12_017_Handle_t x12device[3];
  STEPPER_Config_t stepperConfig;
  STEPPER_Handle_t stepper[12];
  STEPBOARD_Config_t stepBoardConfig;

  /* get default configurations */
  McuX12_017_GetDefaultConfig(&x12config);
  STEPPER_GetDefaultConfig(&stepperConfig);
  STEPBOARD_GetDefaultConfig(&stepBoardConfig);

  /* -------- X12.017 devices: three on each board -------------- */
  /* initialize first X12.017 */
  /* DRV_RESET: PIO0_0 */
  x12config.hasReset = true;
  x12config.hw_reset.gpio = GPIO;
  x12config.hw_reset.port = 0U;
  x12config.hw_reset.pin  = 0U;

  /* M0_DIR: PIO0_16 */
  /* (1, 0, 1) */
  x12config.motor[X12_017_M0].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M0].hw_dir.port = 0U;
  x12config.motor[X12_017_M0].hw_dir.pin  = 16U;

  /* M0_STEP: PIO0_17 */
  x12config.motor[X12_017_M0].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M0].hw_step.port = 0U;
  x12config.motor[X12_017_M0].hw_step.pin  = 17U;


  /* M1_DIR: PIO0_18 */
  x12config.motor[X12_017_M1].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M1].hw_dir.port = 0U;
  x12config.motor[X12_017_M1].hw_dir.pin  = 18U;

  /* M1_STEP: PIO0_19 */
  x12config.motor[X12_017_M1].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M1].hw_step.port = 0U;
  x12config.motor[X12_017_M1].hw_step.pin  = 19U;


  /* M2_DIR: PIO0_20 */
  x12config.motor[X12_017_M2].isInverted = true;
  x12config.motor[X12_017_M2].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M2].hw_dir.port = 0U;
  x12config.motor[X12_017_M2].hw_dir.pin  = 20U;

  /* M2_STEP: PIO0_21 */
  x12config.motor[X12_017_M2].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M2].hw_step.port = 0U;
  x12config.motor[X12_017_M2].hw_step.pin  = 21U;


  /* M3_DIR: PIO0_22 */
  x12config.motor[X12_017_M3].isInverted = true;
  x12config.motor[X12_017_M3].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M3].hw_dir.port = 0U;
  x12config.motor[X12_017_M3].hw_dir.pin  = 22U;

  /* M3_STEP: PIO0_23 */
  x12config.motor[X12_017_M3].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M3].hw_step.port = 0U;
  x12config.motor[X12_017_M3].hw_step.pin  = 23U;

  x12device[0] = McuX12_017_InitDevice(&x12config);



  /* initialize second X12.017 */
  x12config.hasReset = false; /* second device shares the reset line from the first */
  /* M4_DIR: PIO0_24 */
  x12config.motor[X12_017_M0].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M0].hw_dir.port = 0U;
  x12config.motor[X12_017_M0].hw_dir.pin  = 24U;

  /* M4_STEP: PIO0_25 */
  x12config.motor[X12_017_M0].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M0].hw_step.port = 0U;
  x12config.motor[X12_017_M0].hw_step.pin  = 25U;

  /* M5_DIR: PIO0_26 */
  x12config.motor[X12_017_M1].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M1].hw_dir.port = 0U;
  x12config.motor[X12_017_M1].hw_dir.pin  = 26U;

  /* M5_STEP: PIO0_27 */
  x12config.motor[X12_017_M1].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M1].hw_step.port = 0U;
  x12config.motor[X12_017_M1].hw_step.pin  = 27U;

  /* M6_DIR: PIO0_28 */
  x12config.motor[X12_017_M2].isInverted = true;
  x12config.motor[X12_017_M2].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M2].hw_dir.port = 0U;
  x12config.motor[X12_017_M2].hw_dir.pin  = 28U;

  /* M6_STEP: PIO0_29 */
  x12config.motor[X12_017_M2].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M2].hw_step.port = 0U;
  x12config.motor[X12_017_M2].hw_step.pin  = 29U;

  /* M7_DIR: PIO0_10 */
  x12config.motor[X12_017_M3].isInverted = true;
  x12config.motor[X12_017_M3].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M3].hw_dir.port = 0U;
  x12config.motor[X12_017_M3].hw_dir.pin  = 10U;

  /* M7_STEP: PIO0_11 */
  x12config.motor[X12_017_M3].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M3].hw_step.port = 0U;
  x12config.motor[X12_017_M3].hw_step.pin  = 11U;

  x12device[1] = McuX12_017_InitDevice(&x12config);



  /* initialize third X12.017 */
  x12config.hasReset = false; /* third device shares the reset line from the first */
  /* M8_DIR: PIO0_12 */
  x12config.motor[X12_017_M0].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M0].hw_dir.port = 0U;
  x12config.motor[X12_017_M0].hw_dir.pin  = 12U;

  /* M8_STEP: PIO0_13 */
  x12config.motor[X12_017_M0].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M0].hw_step.port = 0U;
  x12config.motor[X12_017_M0].hw_step.pin  = 13U;

  /* M9_DIR: PIO0_14 */
  x12config.motor[X12_017_M1].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M1].hw_dir.port = 0U;
  x12config.motor[X12_017_M1].hw_dir.pin  = 14U;

  /* M9_STEP: PIO0_15 */
  x12config.motor[X12_017_M1].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M1].hw_step.port = 0U;
  x12config.motor[X12_017_M1].hw_step.pin  = 15U;

  /* M10_DIR: PIO0_07 */
  x12config.motor[X12_017_M2].isInverted = true;
  x12config.motor[X12_017_M2].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M2].hw_dir.port = 0U;
  x12config.motor[X12_017_M2].hw_dir.pin  = 07U;

  /* M10_STEP: PIO0_06 */
  x12config.motor[X12_017_M2].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M2].hw_step.port = 0U;
  x12config.motor[X12_017_M2].hw_step.pin  = 06U;

  /* M11_DIR: PIO0_04 */
  x12config.motor[X12_017_M3].isInverted = true;
  x12config.motor[X12_017_M3].hw_dir.gpio = GPIO;
  x12config.motor[X12_017_M3].hw_dir.port = 0U;
  x12config.motor[X12_017_M3].hw_dir.pin  = 04U;

  /* M11_STEP: PIO0_01 */
  x12config.motor[X12_017_M3].hw_step.gpio = GPIO;
  x12config.motor[X12_017_M3].hw_step.port = 0U;
  x12config.motor[X12_017_M3].hw_step.pin  = 01U;

  x12device[2] = McuX12_017_InitDevice(&x12config);




  /* setup the 12 stepper motors */
  x12Steppers[0].x12device = x12device[0];
  x12Steppers[0].x12motor = X12_017_M1;
  x12Steppers[1].x12device = x12device[0];
  x12Steppers[1].x12motor = X12_017_M0;
  x12Steppers[2].x12device = x12device[0];
  x12Steppers[2].x12motor = X12_017_M3;
  x12Steppers[3].x12device = x12device[0];
  x12Steppers[3].x12motor = X12_017_M2;

  x12Steppers[4].x12device = x12device[1];
  x12Steppers[4].x12motor = X12_017_M3;
  x12Steppers[5].x12device = x12device[1];
  x12Steppers[5].x12motor = X12_017_M2;
  x12Steppers[6].x12device = x12device[1];
  x12Steppers[6].x12motor = X12_017_M1;
  x12Steppers[7].x12device = x12device[1];
  x12Steppers[7].x12motor = X12_017_M0;

  x12Steppers[8].x12device = x12device[2];
  x12Steppers[8].x12motor = X12_017_M3;
  x12Steppers[9].x12device = x12device[2];
  x12Steppers[9].x12motor = X12_017_M2;
  x12Steppers[10].x12device = x12device[2];
  x12Steppers[10].x12motor = X12_017_M1;
  x12Steppers[11].x12device = x12device[2];
  x12Steppers[11].x12motor = X12_017_M0;


#if PL_CONFIG_USE_MAG_SENSOR
  x12Steppers[0].mag = MAG_MAG1;
  x12Steppers[1].mag = MAG_MAG0;

  x12Steppers[2].mag = MAG_MAG1;
  x12Steppers[3].mag = MAG_MAG0;

  x12Steppers[4].mag = MAG_MAG1;
  x12Steppers[5].mag = MAG_MAG0;

  x12Steppers[6].mag = MAG_MAG1;
  x12Steppers[7].mag = MAG_MAG0;

  x12Steppers[8].mag = MAG_MAG1;
  x12Steppers[9].mag = MAG_MAG0;

  x12Steppers[10].mag = MAG_MAG1;
  x12Steppers[11].mag = MAG_MAG0;
#endif
  /* initialize stepper, 4 for each X12 driver */
  stepperConfig.stepFn = X12_SingleStep;

  stepperConfig.getBacklashFn = X12_GetBacklashSteps;

  stepperConfig.device = &x12Steppers[0];
  stepperConfig.coord.clock = 0;
  stepperConfig.coord.motor = 0;
  stepper[0] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[1];
  stepperConfig.coord.clock = 0;
  stepperConfig.coord.motor = 1;
  stepper[1] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[2];
  stepperConfig.coord.clock = 1;
  stepperConfig.coord.motor = 0;
  stepper[2] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[3];
  stepperConfig.coord.clock = 1;
  stepperConfig.coord.motor = 1;
  stepper[3] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[4];
  stepperConfig.coord.clock = 2;
  stepperConfig.coord.motor = 0;
  stepper[4] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[5];
  stepperConfig.coord.clock = 2;
  stepperConfig.coord.motor = 1;
  stepper[5] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[6];
  stepperConfig.coord.clock = 3;
  stepperConfig.coord.motor = 0;
  stepper[6] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[7];
  stepperConfig.coord.clock = 3;
  stepperConfig.coord.motor = 1;
  stepper[7] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[8];
  stepperConfig.coord.clock = 4;
  stepperConfig.coord.motor = 0;
  stepper[8] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[9];
  stepperConfig.coord.clock = 4;
  stepperConfig.coord.motor = 1;
  stepper[9] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[10];
  stepperConfig.coord.clock = 5;
  stepperConfig.coord.motor = 0;
  stepper[10] = STEPPER_InitDevice(&stepperConfig);

  stepperConfig.device = &x12Steppers[11];
  stepperConfig.coord.clock = 5;
  stepperConfig.coord.motor = 1;
  stepper[11] = STEPPER_InitDevice(&stepperConfig);

  /* setup board */
  stepBoardConfig.addr = RS485_GetAddress();
  stepBoardConfig.enabled = true;

  for(int i=0; i<STEPBOARD_NOF_CLOCKS*STEPPER_NOF_CLOCK_MOTORS; i++) {
  	Coord_t coord;

  	coord = STEPPER_GetCoord(stepper[i]);
  	stepBoardConfig.stepper[coord.clock][coord.motor] = stepper[i];
  }

  //stepBoardConfig.stepper[0][0] = stepper[0]; // 0 0 0
  //stepBoardConfig.stepper[0][1] = stepper[1]; // 0 0 1
  //stepBoardConfig.stepper[1][0] = stepper[3]; // 1 0 0
  //stepBoardConfig.stepper[1][1] = stepper[2]; // 1 0 1
  //stepBoardConfig.stepper[2][0] = stepper[4];
  //stepBoardConfig.stepper[2][1] = stepper[5];
  //stepBoardConfig.stepper[3][0] = stepper[6];
  //stepBoardConfig.stepper[3][1] = stepper[7];

  MATRIX_Boards[0] = STEPBOARD_InitDevice(&stepBoardConfig);

  McuX12_017_ResetDriver(x12device[0]); /* shared reset line with second device */
}
#endif /* PL_CONFIG_USE_X12_STEPPER */

void MATRIX_Init(void) {
#if PL_CONFIG_USE_X12_STEPPER
  InitSteppers();
#endif
#if PL_CONFIG_USE_STEPPER
  /* ---------------------------------------------------------- */
  /* default */
  STEPBOARD_SetBoard(MATRIX_Boards[0]);
  /* ---------------------------------------------------------- */
#endif

#if PL_CONFIG_IS_MASTER
  /* initialize matrix */
  //MATRIX_DrawAllClockHands(0, 0);
  //MATRIX_DrawAllClockDelays(0, 0);
  //MATRIX_DrawAllMoveMode(STEPPER_MOVE_MODE_SHORT, STEPPER_MOVE_MODE_SHORT);
  //uint8_t buf[McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE];

      /*  matrix q <x> <y> <z> a <angle> <delay> <mode> */
      //McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"matrix s 0 0 0 2000");

      //(void)RS485_SendCommand(clockMatrix[0][0].addr, buf, 1000, true, 1);
  //MATRIX_CopyMatrix(&prevMatrix, &matrix); /* make backup */
  MATRIX_CopyMatrix();
#else
  //Startup
  //MATRIX_MoveAllto(10000, 360);
#endif
#if PL_CONFIG_USE_STEPPER
  if (xTaskCreate(
      MatrixQueueTask,  /* pointer to the task */
      "StepperQueue", /* task name for kernel awareness debugging */
      800/sizeof(StackType_t), /* task stack size */
      (void*)NULL, /* optional task startup argument */
      tskIDLE_PRIORITY+3,  /* initial priority */
      NULL /* optional task handle to create */
    ) != pdPASS)
  {
    for(;;){} /* error! probably out of memory */
  }
#endif
}

#endif /* PL_CONFIG_USE_MATRIX */

