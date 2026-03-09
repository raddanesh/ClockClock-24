#ifndef MATRIXCONFIG_H_
#define MATRIXCONFIG_H_

#include "platform.h"
#include <stdbool.h>
#include <stdint.h>
#if PL_CONFIG_USE_X12_STEPPER
  #include "McuX12_017.h"
#endif
#if PL_CONFIG_USE_MAG_SENSOR
  #include "magnets.h"
#endif
#include "StepperBoard.h"

#if PL_CONFIG_IS_MASTER

/* configuration for master: */
#define MATRIX_NOF_BOARDS         (1)  /* total number of boards in matrix */
#define MATRIX_NOF_CLOCKS_X       (2)  /* number of clocks in x (horizontal) direction */
#define MATRIX_NOF_CLOCKS_Y       (3)   /* number of clocks in y (vertical) direction */
#define MATRIX_NOF_CLOCKS_Z       (2)   /* number of clocks in z direction */

typedef enum {
  BOARD_ADDR_00=0x0A, /* first board top board */
  BOARD_ADDR_01=0x21,
  BOARD_ADDR_02=0x22,

  BOARD_ADDR_05=0x25, /* second row, first board */
  BOARD_ADDR_06=0x26,
  BOARD_ADDR_07=0x27,
} MATRIX_BOARD_ADDR_e;

typedef struct {
  uint8_t addr; /* RS-485 address */
  uint8_t nr;   /* clock number, 0..3 */
  struct {
    uint8_t x, y; /* coordinates on the board */
  } board;
  bool enabled; /* if enabled or not */
} MatrixClock_t;

//extern const MatrixClock_t clockMatrix[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y];

typedef struct {
  bool enabled;
  uint8_t addr;
} MATRIX_BoardList_t;

typedef struct {
  int16_t angleMap[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y][MATRIX_NOF_CLOCKS_Z]; /* two hands per clock */
  int8_t delayMap[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y][MATRIX_NOF_CLOCKS_Z]; /* map of clocks with their speed delay */
  STEPPER_MoveMode_e moveMap[MATRIX_NOF_CLOCKS_X][MATRIX_NOF_CLOCKS_Y][MATRIX_NOF_CLOCKS_Z];
} MATRIX_Matrix_t;

typedef struct {
  int16_t angle; /* absolute angle for clock hand position */
} MHand_t;

typedef struct {
  MHand_t hands[2]; /* each clock has two hands */
} MClock_t;

typedef struct {
	MClock_t digit[2][3]; /* a digit is built by 3 (vertical) and 2 (horizontal) clocks */
} MClockDigit_t;

//extern const MATRIX_BoardList_t MATRIX_BoardList[MATRIX_NOF_BOARDS];
#elif PL_CONFIG_IS_CLIENT /* only single board with 4 clocks */
  #define MATRIX_NOF_CLOCKS_X       (2)  /* number of clocks in x (horizontal) direction */
  #define MATRIX_NOF_CLOCKS_Y       (3)  /* number of clocks in y (vertical) direction */
  #define MATRIX_NOF_CLOCKS_Z       (2)  /* number of clocks in z direction */
  #define MATRIX_NOF_BOARDS         (1)  /* number of boards in matrix */

#if PL_CONFIG_USE_STEPPER
typedef struct {
  McuX12_017_Handle_t x12device;
  McuX12_017_Motor_e x12motor;
#if PL_CONFIG_USE_MAG_SENSOR
  MAG_MagSensor_e mag;
#endif
} X12_Stepper_t;
#endif

#endif /* PL_CONFIG_IS_MASTER */

#define STEPPER_HAND_ZERO_DELAY     (6)

/* settings for a single board: */
#define MATRIX_NOF_CLOCKS_BOARD_X (2)  /* number of clocks on PCB in x direction */
#define MATRIX_NOF_CLOCKS_BOARD_Y (3)  /* number of clocks on PCB in y direction */
#define MATRIX_NOF_CLOCKS_BOARD_Z (2)  /* number of clocks on PCB in z direction */

#define MATRIX_NOF_BOARDS_Y       (MATRIX_NOF_CLOCKS_Y/MATRIX_NOF_CLOCKS_BOARD_Y) /* number of boards in Y (rows) direction */
#define MATRIX_NOF_BOARDS_X       (MATRIX_NOF_CLOCKS_X/MATRIX_NOF_CLOCKS_BOARD_X) /* number of boards in X (columns) direction */

#endif /* MATRIXCONFIG_H_ */
