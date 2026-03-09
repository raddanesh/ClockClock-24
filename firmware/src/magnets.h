#ifndef MAGNETS_H_
#define MAGNETS_H_

#include <stdbool.h>
#include "McuShell.h"

uint8_t MAG_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io);

typedef enum {
  MAG_MAG0,
  MAG_MAG1,
  MAG_NOF_MAG
} MAG_MagSensor_e;

bool MAG_IsTriggered(MAG_MagSensor_e sensor);

void MAG_Deinit(void);
void MAG_Init(void);

#endif /* MAGNETS_H_ */
