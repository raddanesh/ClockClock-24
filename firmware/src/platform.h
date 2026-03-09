#ifndef PLATFORM_H_
#define PLATFORM_H_

#define PL_CONFIG_IS_MASTER         (1) /* otherwise it is the client  */
#define PL_CONFIG_IS_CLIENT         (!PL_CONFIG_IS_MASTER) /* otherwise it is the master */

#define PL_CONFIG_USE_MATRIX        (1)
#define PL_CONFIG_USE_WDT           (0) /* if using watchdog timer */
#define PL_CONFIG_USE_SHELL         (1) /* use command line shell */
#define PL_CONFIG_USE_RS485         (1 && PL_CONFIG_USE_SHELL) /* RS-485 connection, 1: enabled, 0: disabled: it requires the shell to parse the commands */

/* client only: */
#define PL_CONFIG_USE_MAG_SENSOR    (1 && PL_CONFIG_IS_CLIENT) /* using magnets and hall sensors */
#define PL_CONFIG_USE_STEPPER       (1 && PL_CONFIG_IS_CLIENT) /* enable stepper motors */
#define PL_CONFIG_USE_X12_STEPPER   (1 && PL_CONFIG_USE_STEPPER) /* X12 stepper motors */

#define PL_CONFIG_USE_SHELL_UART    (1 && PL_CONFIG_IS_MASTER) /* using UART for USB-CDC to host */

void PL_Init(void);

#endif /* PLATFORM_H_ */
