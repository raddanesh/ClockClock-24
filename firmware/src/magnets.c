#include "platform.h"
#if PL_CONFIG_USE_MAG_SENSOR
#include "magnets.h"
#include "McuGPIO.h"
#include "McuUtility.h"

/* Hall_M0: PIO0_0 */
#define MAG0_GPIO       GPIO
#define MAG0_PORT       0U
#define MAG0_PIN        8U
#define MAG0_IOCON      IOCON_INDEX_PIO0_0

/* Hall_M1: PIO0_1 */
#define MAG1_GPIO       GPIO
#define MAG1_PORT       0U
#define MAG1_PIN        9U
#define MAG1_IOCON      IOCON_INDEX_PIO0_1

static McuGPIO_Handle_t MAG_MagSensor[MAG_NOF_MAG];

bool MAG_IsTriggered(MAG_MagSensor_e sensor) {
  return McuGPIO_IsLow(MAG_MagSensor[sensor]);
}

static uint8_t PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[64];
  unsigned char status[16];

  McuShell_SendStatusStr((unsigned char*)"mag", (unsigned char*)"Magnetic hall sensor settings\r\n", io->stdOut);

  for(int i=MAG_MAG0; i<MAG_NOF_MAG; i++) {
    McuGPIO_GetPinStatusString(MAG_MagSensor[i], buf, sizeof(buf));
    McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"\r\n");
    McuUtility_strcpy(status, sizeof(status), (unsigned char*)"  Mag[");
    McuUtility_strcatNum32u(status, sizeof(status), i);
    McuUtility_chcat(status, sizeof(status), ']');
    McuShell_SendStatusStr(status, buf, io->stdOut);
  }
  return ERR_OK;
}

static uint8_t PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"mag", (unsigned char*)"Group of magnet/hall sensor commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Print help or status information\r\n", io->stdOut);
  return ERR_OK;
}

uint8_t MAG_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  if (McuUtility_strcmp((char*)cmd, McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, "mag help")==0) {
    *handled = TRUE;
    return PrintHelp(io);
  } else if ((McuUtility_strcmp((char*)cmd, McuShell_CMD_STATUS)==0) || (McuUtility_strcmp((char*)cmd, "mag status")==0)) {
    *handled = TRUE;
    return PrintStatus(io);
  }
  return ERR_OK;
}

void MAG_Deinit(void) {
  for(int i=0; i<MAG_NOF_MAG; i++) {
    MAG_MagSensor[i] = McuGPIO_DeinitGPIO(MAG_MagSensor[i]);
  }
}

#include <stdio.h>
void MAG_Init(void) {
  McuGPIO_Config_t gpioConfig;

  McuGPIO_GetDefaultConfig(&gpioConfig);
  /* internal pull-ups are enabled in Pins Tool! (or use HW pull-ups!) */

  gpioConfig.hw.gpio = MAG0_GPIO;
  gpioConfig.hw.port = MAG0_PORT;
  gpioConfig.hw.pin = MAG0_PIN;
  gpioConfig.hw.iocon = MAG0_IOCON;
  gpioConfig.isInput = true;
  MAG_MagSensor[MAG_MAG0] = McuGPIO_InitGPIO(&gpioConfig);
  McuGPIO_SetPullResistor(MAG_MagSensor[MAG_MAG0], McuGPIO_PULL_UP);

  gpioConfig.hw.gpio = MAG1_GPIO;
  gpioConfig.hw.port = MAG1_PORT;
  gpioConfig.hw.pin = MAG1_PIN;
  gpioConfig.hw.iocon = MAG1_IOCON;
  gpioConfig.isInput = true;
  MAG_MagSensor[MAG_MAG1] = McuGPIO_InitGPIO(&gpioConfig);
  McuGPIO_SetPullResistor(MAG_MagSensor[MAG_MAG1], McuGPIO_PULL_UP);
}

#endif /* PL_CONFIG_USE_MAG_SENSOR */
