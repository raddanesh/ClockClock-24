#include "platform.h"
#if PL_CONFIG_USE_MATRIX && PL_CONFIG_USE_SHELL

#include "matrixShell.h"

#include "matrixconfig.h"
#include "matrix.h" /* if you need MATRIX_GetClock() on master; otherwise guard it */
#include "McuUtility.h"
#include "rs485.h"
#include "stepper.h"
#include "McuWait.h"
#include "Shell.h"

#if PL_CONFIG_USE_WDT
#include "watchdog.h"
#endif
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

extern bool MATRIX_ExecuteQueue;
STEPPER_Handle_t MATRIX_GetStepper(int32_t x, int32_t y, int32_t z);

static uint8_t PrintHelp(const McuShell_StdIOType *io)
{
    McuShell_SendHelpStr((unsigned char *)"matrix", (unsigned char *)"Group of matrix commands\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  help|status", (unsigned char *)"Print help or status information\r\n", io->stdOut);
#if PL_CONFIG_USE_STEPPER
    McuShell_SendHelpStr((unsigned char *)"  stepper status", (unsigned char *)"Print stepper status information\r\n", io->stdOut);
#endif
    McuShell_SendHelpStr((unsigned char *)"  test", (unsigned char *)"Test the stepper on the board\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  12", (unsigned char *)"Set matrix to 12:00 position\r\n", io->stdOut);
#if PL_CONFIG_IS_MASTER
    McuShell_SendHelpStr((unsigned char *)"  demo", (unsigned char *)"General longer demo\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 0", (unsigned char *)"Demo with propeller\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 1", (unsigned char *)"Demo with clap effect\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 2", (unsigned char *)"Demo with changing angles\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 3", (unsigned char *)"Demo with propeller wave\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 4", (unsigned char *)"Demo turning clap\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 5", (unsigned char *)"Demo with squares\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 6", (unsigned char *)"Demo fast clock\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  demo 7", (unsigned char *)"Demo moving hand around\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  test 0", (unsigned char *)"LED and hand test\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  intermezzo <nr>", (unsigned char *)"Play Intermezzo\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  delay <delay>", (unsigned char *)"Set default delay\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  time <time>", (unsigned char *)"Show time\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  temp <temp>", (unsigned char *)"Show temperature\r\n", io->stdOut);
#endif
#if PL_CONFIG_USE_STEPPER
    McuShell_SendHelpStr((unsigned char *)"", (unsigned char *)"<xyz>: coordinate, separated by space, e.g. 0 0 1\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"", (unsigned char *)"<md>: mode (cc, cw, sh), lowercase mode letter is with accel control for start/stop, e.g. Cw\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"", (unsigned char *)"<d>: delay, 0 is no delay\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  r <xyz> <a> <d> <md>", (unsigned char *)"Relative angle move\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  a <xyz> <a> <d> <md>", (unsigned char *)"Absolute angle move\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  q <xyz> <cmd>", (unsigned char *)"Queue a 'r' or 'a' command, e.g. 'matrix q 0 0 0 r 90 8 cc'\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  exq", (unsigned char *)"Execute commands in queue\r\n", io->stdOut);
#if PL_CONFIG_IS_MASTER
    McuShell_SendHelpStr((unsigned char *)"  lastError", (unsigned char *)"Check remotes for last error\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  waitidle", (unsigned char *)"Check remotes for idle state\r\n", io->stdOut);
#endif
#endif
#if PL_CONFIG_USE_MAG_SENSOR
    McuShell_SendHelpStr((unsigned char *)"  zero all", (unsigned char *)"Move all motors to zero position using magnet sensor\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  zero <x> <y> <z>", (unsigned char *)"Move clock to zero position using magnet sensor\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  offs <x> <y> <z> <v>", (unsigned char *)"Set offset value for clock\r\n", io->stdOut);
    McuShell_SendHelpStr((unsigned char *)"  offs 12", (unsigned char *)"Calculate offset from 12-o-clock for all\r\n", io->stdOut);
#endif
    return ERR_OK;
}

static uint8_t PrintStatus(const McuShell_StdIOType *io)
{
    uint8_t buf[32];

    McuShell_SendStatusStr((unsigned char *)"matrix", (unsigned char *)"Matrix settings\r\n", io->stdOut);
    McuUtility_strcpy(buf, sizeof(buf), (unsigned char *)"x*y: ");
    McuUtility_strcatNum8u(buf, sizeof(buf), MATRIX_NOF_CLOCKS_X);
    McuUtility_chcat(buf, sizeof(buf), '*');
    McuUtility_strcatNum8u(buf, sizeof(buf), MATRIX_NOF_CLOCKS_Y);
    McuUtility_strcat(buf, sizeof(buf), (unsigned char *)"\r\n");
    McuShell_SendStatusStr((unsigned char *)"  clocks", buf, io->stdOut);

    if (MATRIX_ExecuteQueue)
    {
        McuUtility_strcpy(buf, sizeof(buf), (unsigned char *)"execute: yes");
    }
    else
    {
        McuUtility_strcpy(buf, sizeof(buf), (unsigned char *)"execute: no");
    }
    McuUtility_strcat(buf, sizeof(buf), (unsigned char *)"\r\n");
    McuShell_SendStatusStr((unsigned char *)"  queue", buf, io->stdOut);

    return ERR_OK;
}

#if PL_CONFIG_USE_STEPPER
static uint8_t PrintStepperStatus(const McuShell_StdIOType *io)
{
    uint8_t buf[128];
    uint8_t statusStr[16];

    for (int x = 0; x < MATRIX_NOF_CLOCKS_X; x++)
    {
        for (int y = 0; y < MATRIX_NOF_CLOCKS_Y; y++)
        {
            for (int z = 0; z < MATRIX_NOF_CLOCKS_Z; z++)
            {
                STEPPER_Handle_t stepper;
                uint8_t addr, nr;

                stepper = MATRIX_GetStepper(x, y, z);
                addr = MATRIX_GetAddress(x, y, z);
                nr = MATRIX_GetPos(x, y, z);
                buf[0] = '\0';
                McuUtility_strcat(buf, sizeof(buf), (unsigned char *)"addr:0x");
                McuUtility_strcatNum8Hex(buf, sizeof(buf), addr);
                McuUtility_strcat(buf, sizeof(buf), (unsigned char *)", nr:");
                McuUtility_strcatNum8u(buf, sizeof(buf), nr);
                McuUtility_strcat(buf, sizeof(buf), (unsigned char *)", Stepper ");
                STEPPER_StrCatStatus(stepper, buf, sizeof(buf));
                McuUtility_strcat(buf, sizeof(buf), (unsigned char *)"\r\n");

                McuUtility_strcpy(statusStr, sizeof(statusStr), (unsigned char *)"  M[");
                McuUtility_strcatNum8u(statusStr, sizeof(statusStr), x);
                McuUtility_chcat(statusStr, sizeof(statusStr), ',');
                McuUtility_strcatNum8u(statusStr, sizeof(statusStr), y);
                McuUtility_chcat(statusStr, sizeof(statusStr), ',');
                McuUtility_strcatNum8u(statusStr, sizeof(statusStr), z);
                McuUtility_strcat(statusStr, sizeof(statusStr), (unsigned char *)"]");
                McuShell_SendStatusStr(statusStr, buf, io->stdOut);
            }
        }
    } /* for */
    return ERR_OK;
}

static uint8_t ParseMatrixCommand(const unsigned char *cmd, int32_t *xp, int32_t *yp, int32_t *zp, int32_t *vp, uint8_t *dp, STEPPER_MoveMode_e *modep, bool *speedUpp, bool *slowDownp)
{
    /* parse a string like <x> <y> <z> <v> <d> <md> */
    int32_t x, y, z, v, d;

    if (McuUtility_xatoi(&cmd, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&cmd, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&cmd, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z && McuUtility_xatoi(&cmd, &v) == ERR_OK && McuUtility_xatoi(&cmd, &d) == ERR_OK && d >= 0)
    {
        if (*cmd == ' ')
        {
            cmd++;
        }
        if (*cmd == '\0')
        { /* mode is optional, set it to defaults */
            *modep = STEPPER_MOVE_MODE_SHORT;
            *speedUpp = *slowDownp = true;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"cw") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CW;
            *speedUpp = *slowDownp = true;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"Cw") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CW;
            *speedUpp = false;
            *slowDownp = true;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"cW") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CW;
            *speedUpp = true;
            *slowDownp = false;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"CW") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CW;
            *speedUpp = *slowDownp = false;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"cc") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CCW;
            *speedUpp = *slowDownp = true;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"Cc") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CCW;
            *speedUpp = false;
            *slowDownp = true;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"cC") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CCW;
            *speedUpp = true;
            *slowDownp = false;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"CC") == 0)
        {
            *modep = STEPPER_MOVE_MODE_CCW;
            *speedUpp = *slowDownp = false;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"sh") == 0)
        {
            *modep = STEPPER_MOVE_MODE_SHORT;
            *speedUpp = *slowDownp = true;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"Sh") == 0)
        {
            *modep = STEPPER_MOVE_MODE_SHORT;
            *speedUpp = false;
            *slowDownp = true;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"sH") == 0)
        {
            *modep = STEPPER_MOVE_MODE_SHORT;
            *speedUpp = true;
            *slowDownp = false;
        }
        else if (McuUtility_strcmp((char *)cmd, (char *)"SH") == 0)
        {
            *modep = STEPPER_MOVE_MODE_SHORT;
            *speedUpp = *slowDownp = false;
        }
        else
        {
            return ERR_FAILED;
        }
        *xp = x;
        *yp = y;
        *zp = z;
        *vp = v;
        *dp = d;
        return ERR_OK;
    }
    return ERR_FAILED;
}
#endif

uint8_t MATRIX_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io)
{
    const unsigned char *p;

#if PL_CONFIG_USE_STEPPER
    uint8_t res = ERR_OK;
    int32_t x, y, z, v, s;
    uint8_t d;
    bool speedUp, slowDown;
    STEPPER_MoveMode_e mode;
#endif

    if (McuUtility_strcmp((char *)cmd, McuShell_CMD_HELP) == 0 || McuUtility_strcmp((char *)cmd, "matrix help") == 0)
    {
        *handled = true;
        return PrintHelp(io);
    }
    else if ((McuUtility_strcmp((char *)cmd, McuShell_CMD_STATUS) == 0) || (McuUtility_strcmp((char *)cmd, "matrix status") == 0))
    {
        *handled = true;
        return PrintStatus(io);
#if PL_CONFIG_USE_STEPPER
    }
    else if ((McuUtility_strcmp((char *)cmd, McuShell_CMD_STATUS) == 0) || (McuUtility_strcmp((char *)cmd, "matrix stepper status") == 0))
    {
        *handled = true;
        return PrintStepperStatus(io);
#endif
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix all ", sizeof("matrix all ") - 1) == 0)
    {
        *handled = true;
#if PL_CONFIG_USE_STEPPER
        int32_t degree;
        p = cmd + sizeof("matrix all ") - 1;
        if (McuUtility_xatoi(&p, &degree) == ERR_OK)
        {
            return MATRIX_MoveAllto(10000, degree);
        }
        else
        {
            return ERR_FAILED;
        }
#else
        return MATRIX_MoveAllto12(10000, io);
#endif
#if PL_CONFIG_IS_MASTER
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo") == 0)
    {
        *handled = true;

        // uint8_t buf[McuShell_CONFIG_DEFAULT_SHELL_BUFFER_SIZE];

        /*  matrix q <x> <y> <z> a <angle> <delay> <mode> */
        // McuUtility_strcpy(buf, sizeof(buf), (unsigned char*)"matrix s 0 0 0 2000");

        //(void)RS485_SendCommand(MATRIX_GetClock(0, 0).addr, buf, 1000, true, 1); /* queue the command for the remote boards */
        // return ERR_OK;
        return MATRIX_Demo(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 0") == 0)
    {
        *handled = true;
        return MATRIX_Demo0(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 1") == 0)
    {
        *handled = true;
        return MATRIX_Demo1(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 2") == 0)
    {
        *handled = true;
        return MATRIX_Demo2(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 3") == 0)
    {
        *handled = true;
        return MATRIX_Demo3(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 4") == 0)
    {
        *handled = true;
        return MATRIX_Demo4(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 5") == 0)
    {
        *handled = true;
        return MATRIX_Demo5(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 6") == 0)
    {
        *handled = true;
        return MATRIX_Demo6(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix demo 7") == 0)
    {
        *handled = true;
        return MATRIX_Demo7(io);
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix test 0") == 0)
    {
        *handled = true;
        return MATRIX_Test0(io);
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix digit ", sizeof("matrix digit ") - 1) == 0)
    {
        uint8_t dg;

        *handled = TRUE;
        p = cmd + sizeof("matrix digit ") - 1;
        if (McuUtility_ScanDecimal8uNumber(&p, &dg) == ERR_OK)
        {
            return MATRIX_ShowDigit(dg);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix intermezzo ", sizeof("matrix intermezzo ") - 1) == 0)
    {
        uint8_t nr;

        *handled = TRUE;
        p = cmd + sizeof("matrix intermezzo ") - 1;
        if (McuUtility_ScanDecimal8uNumber(&p, &nr) == ERR_OK)
        {
            return MATRIX_Intermezzo(&nr);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix delay ", sizeof("matrix delay ") - 1) == 0)
    {
        uint8_t delay;

        *handled = TRUE;
        p = cmd + sizeof("matrix delay ") - 1;
        if (McuUtility_ScanDecimal8uNumber(&p, &delay) == ERR_OK)
        {
            return MATRIX_DrawAllClockDelays(delay, delay);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix time ", sizeof("matrix time ") - 1) == 0)
    {
        uint8_t hour, minute, second, hsec;

        *handled = TRUE;
        p = cmd + sizeof("matrix time ") - 1;
        if (McuUtility_ScanTime(&p, &hour, &minute, &second, &hsec) == ERR_OK)
        {
            return MATRIX_ShowTime(hour, minute);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix temp ", sizeof("matrix temp ") - 1) == 0)
    {
        uint8_t temperature;

        *handled = TRUE;
        p = cmd + sizeof("matrix time ") - 1;
        if (McuUtility_ScanDecimal8uNumber(&p, &temperature) == ERR_OK)
        {
            return MATRIX_ShowTemperature(temperature);
        }
        else
        {
            return ERR_FAILED;
        }
#endif /* PL_CONFIG_IS_MASTER */
#if PL_CONFIG_USE_STEPPER
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix r ", sizeof("matrix r ") - 1) == 0)
    { /* "matrix r <x> <y> <z> <a> <d> <md> " */
        *handled = TRUE;
        res = ParseMatrixCommand(cmd + sizeof("matrix r ") - 1, &x, &y, &z, &v, &d, &mode, &speedUp, &slowDown);
        if (res == ERR_OK)
        {
            STEPPER_MoveClockDegreeRel(MATRIX_GetStepper(x, y, z), v, mode, d, speedUp, slowDown);
            STEPPER_StartTimer();
            return ERR_OK;
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix s ", sizeof("matrix s ") - 1) == 0)
    { /* "matrix s <x> <y> <z> <steps> [<delay>]" */
        *handled = TRUE;

        p = cmd + sizeof("matrix s ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z && McuUtility_xatoi(&p, &s) == ERR_OK)
        {
            /* optional delay parameter */
            int32_t delayParam = 6;                  /* default if not provided */
            (void)McuUtility_xatoi(&p, &delayParam); /* ignore error: delay is optional */

            if (delayParam < 0)
            {
                delayParam = 0;
            }
            else if (delayParam > 255)
            {
                delayParam = 255;
            }

            STEPPER_MoveMotorStepsRel(MATRIX_GetStepper(x, y, z), s, (uint16_t)delayParam);
            STEPPER_StartTimer();
            return ERR_OK;
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix a ", sizeof("matrix a ") - 1) == 0)
    { /* "matrix a <x> <y> <z> <a> <d> <md> " */
        *handled = TRUE;
        res = ParseMatrixCommand(cmd + sizeof("matrix a ") - 1, &x, &y, &z, &v, &d, &mode, &speedUp, &slowDown);
        if (res == ERR_OK)
        {
            STEPPER_MoveClockDegreeAbs(MATRIX_GetStepper(x, y, z), v, mode, d, speedUp, slowDown);
            STEPPER_StartTimer();
            return ERR_OK;
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix q ", sizeof("matrix q ") - 1) == 0)
    {
        unsigned char *ptr, *data;
        STEPPER_Handle_t stepper;

        *handled = TRUE;
        p = cmd + sizeof("matrix q ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z)
        {
            data = (unsigned char *)p;
            while (*data == ' ')
            {
                data++;
            }
            ptr = pvPortMalloc(McuUtility_strlen((char *)data) + 1);
            if (ptr == NULL)
            {
                return ERR_FAILED;
            }
            stepper = MATRIX_GetStepper(x, y, z);
            strcpy((char *)ptr, (char *)data); /* copy command */
            if (xQueueSendToBack(STEPPER_GetQueue(stepper), &ptr, pdMS_TO_TICKS(100)) != pdTRUE)
            {
                return ERR_FAILED;
            }
            return ERR_OK;
        }
        else
        {
            return ERR_FAILED;
        }
#endif
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix exq") == 0)
    {
        MATRIX_ExecuteQueue = true;
        *handled = TRUE;
#if PL_CONFIG_IS_MASTER
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix lastError") == 0)
    {
        *handled = TRUE;
        return MATRIX_CheckRemoteLastError();
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix waitidle") == 0)
    {
        *handled = TRUE;
        return MATRIX_WaitForIdle(10000);
#endif
#if PL_CONFIG_USE_MAG_SENSOR
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix zero all") == 0)
    {
        *handled = TRUE;
        return MATRIX_ZeroAllHands();
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix blsh ", sizeof("matrix blsh ") - 1) == 0)
    {
        *handled = TRUE;
        int16_t backlashSteps;

        p = cmd + sizeof("matrix blsh ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z && McuUtility_ScanDecimal16sNumber(&p, &backlashSteps) == ERR_OK && backlashSteps >= 0)
        {
            return MATRIX_SetBacklashSteps(x, y, z, backlashSteps);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix zero ", sizeof("matrix zero ") - 1) == 0)
    {
        *handled = TRUE;
        p = cmd + sizeof("matrix zero ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z)
        {
            STEPPER_Handle_t stepper;
            X12_Stepper_t *s;

            /* optional delay parameter */
            int32_t delayParam = STEPPER_HAND_ZERO_DELAY; /* default if not provided */
            (void)McuUtility_xatoi(&p, &delayParam);      /* ignore error: delay is optional */
            if (delayParam < 0)
            {
                delayParam = 0;
            }
            else if (delayParam > 65535)
            {
                delayParam = 65535;
            }

            stepper = MATRIX_GetStepper(x, y, z);

            s = STEPPER_GetDevice(stepper);
            McuX12_017_ResetDriver(s->x12device);

            res = MATRIX_STEPPER_ZeroHand(stepper,
                                          NVMC_GetStepperZeroOffset(MATRIX_GetClockPosition(x, y), z),
                                          (uint16_t)delayParam);
            if (res != ERR_OK)
            {
                McuShell_SendStr((unsigned char *)"failed to find magnet/zero position\r\n", io->stdErr);
                return res;
            }
            return ERR_OK;
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix zeroclock ", sizeof("matrix zeroclock ") - 1) == 0)
    {
        *handled = TRUE;
        p = cmd + sizeof("matrix zeroclock ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y)
        {
            return MATRIX_ZeroClock(x, y, io);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix zeroboard") == 0)
    {
        *handled = TRUE;

        return MATRIX_ZeroBoardHands();
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix offset ", sizeof("matrix offset ") - 1) == 0)
    {
        *handled = TRUE;
        int16_t offsets;

        p = cmd + sizeof("matrix offset ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z && McuUtility_ScanDecimal16sNumber(&p, &offsets) == ERR_OK /* && offsets >= 0 */
        )
        {
            return MATRIX_SetOffset(x, y, z, offsets);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix offs 12") == 0)
    {
        *handled = TRUE;
        return MATRIX_SetOffsetFrom12();
    }
    else if (McuUtility_strcmp((char *)cmd, "matrix zerobymid") == 0)
    {
        *handled = TRUE;
        return MATRIX_ZeroMatrixByMid();
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix offs 12 ", sizeof("matrix offs 12 ") - 1) == 0)
    {
        *handled = TRUE;

        p = cmd + sizeof("matrix offs 12 ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z)
        {

            return MATRIX_SetOffsetSteppersFrom12(x, y, z, io);
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix 12 ", sizeof("matrix 12 ") - 1) == 0)
    {
        *handled = TRUE;
        int16_t offset;

        p = cmd + sizeof("matrix 12 ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z && McuUtility_ScanDecimal16sNumber(&p, &offset) == ERR_OK)
        {

            res = MATRIX_AllHandsOff(z);
            if (res == ERR_OK)
            {
                return MATRIX_SetStepper12(x, y, z, offset);
            }
            else
            {
                return ERR_FAILED;
            }
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix reset ", sizeof("matrix reset ") - 1) == 0)
    {
        *handled = TRUE;
        STEPPER_Handle_t st;
        X12_Stepper_t *s;

        p = cmd + sizeof("matrix reset ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z)
        {

            st = MATRIX_GetStepper(x, y, z);
            s = STEPPER_GetDevice(st);

            McuX12_017_ResetDriver(s->x12device);

            return ERR_OK;
        }
        else
        {
            return ERR_FAILED;
        }
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix hoff ", sizeof("matrix hoff ") - 1) == 0)
    {
        *handled = TRUE;

        // p = cmd + sizeof("matrix hoff ")-1;

        res = MATRIX_AllHandsOff(0);
        if (res == ERR_OK)
        {
            return MATRIX_AllHandsOff(1);
        }
        else
        {
            return ERR_FAILED;
        }

        /*
        if (McuUtility_xatoi(&p, &z)==ERR_OK && z>=0 && z<MATRIX_NOF_CLOCKS_Z) {

            return MATRIX_AllHandsOff(z);
        } else {
          return ERR_FAILED;
        }
        */
    }
    else if (McuUtility_strncmp((char *)cmd, "matrix test ", sizeof("matrix test ") - 1) == 0)
    {
        *handled = TRUE;
        uint8_t buf[50];
        STEPPER_Handle_t st;
        int32_t leftPos = 0;
        int32_t rightPos = 400;
        int32_t offset = 0;
        int32_t stepSize = 1;
        bool onRight = false;

        p = cmd + sizeof("matrix test ") - 1;
        if (McuUtility_xatoi(&p, &x) == ERR_OK && x >= 0 && x < MATRIX_NOF_CLOCKS_X && McuUtility_xatoi(&p, &y) == ERR_OK && y >= 0 && y < MATRIX_NOF_CLOCKS_Y && McuUtility_xatoi(&p, &z) == ERR_OK && z >= 0 && z < MATRIX_NOF_CLOCKS_Z)
        {

            st = MATRIX_GetStepper(x, y, z);
            X12_Stepper_t *xs = STEPPER_GetDevice(st);
            McuX12_017_ResetDriver(xs->x12device);

            /* move forward in larger steps to find sensor */
            if (MATRIX_STEPPER_MoveHandOnSensor(st, true, 10, 10000, 10, STEPPER_HAND_ZERO_DELAY) != ERR_OK)
            {
                res = ERR_FAILED;
            }

            /* step back in micro-steps just to leave the sensor */
            if (MATRIX_STEPPER_MoveHandOnSensor(st, false, -1, 10000, 10, STEPPER_HAND_ZERO_DELAY) != ERR_OK)
            {
                res = ERR_FAILED;
            }

            /* step forward in micro-steps just to enter the sensor again */
            if (MATRIX_STEPPER_MoveHandOnSensor(st, true, 1, 10000, 2, STEPPER_HAND_ZERO_DELAY) != ERR_OK)
            {
                res = ERR_FAILED;
            }

            STEPPER_SetPos(st, 0);

            while (abs(rightPos - leftPos) > 10)
            {
                if (MATRIX_STEPPER_MoveHandOnSensor(st, false, stepSize, 10000, 10, STEPPER_HAND_ZERO_DELAY) != ERR_OK)
                {
                    res = ERR_FAILED;
                }

                stepSize = -stepSize;

                if (MATRIX_STEPPER_MoveHandOnSensor(st, true, stepSize, 10000, 2, STEPPER_HAND_ZERO_DELAY) != ERR_OK)
                {
                    res = ERR_FAILED;
                }

                onRight = !onRight;

                if (onRight)
                {
                    rightPos = abs(STEPPER_GetPos(st));
                }
                else
                {
                    leftPos = abs(STEPPER_CLOCK_360_STEPS - STEPPER_GetPos(st));
                }

                STEPPER_SetPos(st, 0);

                /* step back in micro-steps just to leave the sensor */
                // if (MATRIX_STEPPER_MoveHandOnSensor(st, false, -1, 10000, 10, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
                //   res = ERR_FAILED;
                // }

                /* step forward in micro-steps just to enter the sensor again */
                // if (MATRIX_STEPPER_MoveHandOnSensor(st, true, 1, 10000, 2, STEPPER_HAND_ZERO_DELAY)!=ERR_OK) {
                //	res = ERR_FAILED;
                // }

                // onRight = false;
                // leftPos = abs(STEPPER_CLOCK_360_STEPS - STEPPER_GetPos(st));
                // STEPPER_SetPos(st, 0);
            }
        }
        else
        {
            return ERR_FAILED;
        }

        offset = (rightPos + leftPos) / 4;

        if (onRight)
        {
            offset = -offset;
        }

        McuUtility_strcpy(buf, sizeof(buf), (unsigned char *)"\r\n ");
        McuUtility_strcatNum32u(buf, sizeof(buf), offset);
        McuUtility_strcat(buf, sizeof(buf), (unsigned char *)"\r\n");
        McuShell_SendStatusStr((unsigned char *)"Offset", buf, io->stdOut);

        /* here all hands are on the sensor: adjust with offset */
        STEPPER_MoveMotorStepsRel(st, offset, STEPPER_HAND_ZERO_DELAY);
        STEPBOARD_MoveAndWait(STEPBOARD_GetBoard(), 10);
        STEPPER_SetPos(st, 0);

#endif
    }
    return ERR_OK;
}

#endif /* PL_CONFIG_USE_MATRIX && PL_CONFIG_USE_SHELL */