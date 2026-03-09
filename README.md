# ClockClock-24

## LPC845-BRK usage in master and client firmware

This project uses the **NXP LPC845-BRK** as the main MCU platform for both firmware roles in the system. I kept a **single firmware codebase** and used compile-time configuration to switch the board behavior between **master** and **client** modes.

The [LPC845 breakout](https://www.nxp.com/design/design-center/development-boards-and-designs/LPC845-BRK) board provides a powerful and flexible development system for NXP's low end Cortex-M0+-based LPC84x Family of MCUs, delivered in an ultra-low-cost evaluation board. 

This breakout board can be used with a range of development tools, including the MCUXpresso IDE toolchain. The LPCXpresso845 Breakout board is developed by NXP to enable evaluation of and prototyping with the LPC84x family of MCUs.

<p align="center">
  <img src="/PCB/images/LPC845-BRK.png" alt="LPC845-BRK" width="45%" />
</p>

The LPC845 breakout board features an on-board CMSIS-DAP debug and VCOM port, RGB user LEDs, capacitive touch button, user potentiometer and allows an easy prototyping experience with access to 38 LPC845 port pins.

The role selection is controlled through `PL_CONFIG_IS_MASTER` in `platform.h`:

- `PL_CONFIG_IS_MASTER = 1` builds the **master**
- `PL_CONFIG_IS_MASTER = 0` builds the **client**

This approach keeps the low-level platform code shared while allowing each build to enable only the modules needed for its role.

### Why LPC845-BRK

I used the LPC845-BRK because it provides the peripherals needed for this project in a compact Cortex-M0+ platform:

- enough GPIO for stepper-driver control lines and magnetic sensor inputs
- multiple USARTs for separating debug/shell communication from the RS-485 bus
- SCTimer support for motion timing
- internal flash for non-volatile configuration storage
- straightforward integration with the MCUXpresso SDK

The project follows the normal MCUXpresso startup flow:

- `BOARD_InitBootPins()`
- `BOARD_InitBootClocks()`
- `BOARD_InitBootPeripherals()`

After board startup, the firmware enters `APP_Run()`, initializes the platform in `PL_Init()`, and starts FreeRTOS.

### Shared LPC845 platform layer

Both master and client firmware use the same base initialization code in `platform.c`. In `PL_Init()`, the LPC845 initializes the shared system modules, including:

- MCU utility library
- FreeRTOS
- GPIO support
- optional shell/debug UART
- X12.017 stepper driver interface
- stepper control layer
- magnet sensor layer
- RS-485 communication
- non-volatile memory (`nvmc`)
- matrix logic

This gives both firmware roles the same hardware abstraction and startup flow, while feature selection is handled by configuration.

### LPC845-BRK in the master module

In **master mode**, the LPC845 is used as the **system coordinator**. Its job is not to drive the motors directly, but to manage the logical clock matrix and send commands to client boards.

The master firmware is responsible for:

- maintaining the logical matrix state
- converting digits and animations into hand target positions
- sending commands to client boards over **RS-485**
- providing a shell/debug interface over **USART0**

#### Master communication usage

On the master, I used:

- **USART0** for the local shell/debug console
- **USART2** for the RS-485 bus

This allows the master board to stay connected to a PC for debugging or manual commands while also controlling the distributed client nodes.

The shell is enabled only in master mode, so the LPC845 acts as both the local command interface and the RS-485 network controller.

#### Master logic role

The main master logic is implemented in `matrixMaster.c`. In this mode, the LPC845:

- stores the current matrix state
- maps clocks to board addresses and positions
- generates patterns for digits and animations
- forwards movement commands to the correct client board

So in the master build, the LPC845-BRK is mainly used as a **central controller and communication node**.

### LPC845-BRK in the client module

In **client mode**, the LPC845 is used as the **real-time motor-control MCU** for one physical board.

The client firmware is responsible for:

- controlling the local X12.017 driver ICs
- driving all local clock motors
- reading Hall/magnetic sensors for homing
- zeroing and calibrating the hands
- storing local calibration and address data in flash
- receiving commands from the master over **RS-485**

#### Client communication usage

On the client, the LPC845 uses **USART2** for RS-485 communication. Unlike the master, the client does not need the shell/debug console and focuses only on bus-controlled operation.

One useful LPC845 feature I used here is its **hardware-assisted RS-485 direction control**. Instead of manually toggling a GPIO for transmit/receive switching, the firmware uses the USART RTS output as the **RS485_TXRX_EN** signal.

In this design:

- `USART2_TXD` -> `P1_2`
- `USART2_RXD` -> `P1_0`
- `USART2_RTS` -> `P1_1`

This simplifies half-duplex RS-485 handling and reduces timing issues in the communication layer.

#### Client motion-control role

The client-side implementation is mainly in:

- `matrixClient.c`
- `stepper.c`
- `StepperBoard.c`

In this mode, the LPC845 directly controls the board hardware by:

- managing the local 12 motors
- generating step and direction signals for the X12.017 drivers
- using the **SCTimer** for motion timing
- reading magnet sensors to find repeatable zero positions
- applying stored offsets and backlash compensation

So in the client build, the LPC845-BRK is the actual **embedded motion-control node**.

### Internal flash usage

I also used the LPC845 internal flash to store persistent board-specific settings through the `nvmc` module. A reserved flash page is used for non-volatile configuration such as:

- RS-485 board address
- per-stepper zero offsets
- backlash compensation values

This allows each client board to keep its calibration and address after reset or power loss.

### Summary

The LPC845-BRK is used in two different roles in this project:

- In the **master**, it acts as the matrix controller, animation engine, shell host, and RS-485 command source.
- In the **client**, it acts as the real-time motor controller, handling X12.017 stepper driving, sensor-based homing, and local calibration storage.

Using the same LPC845 platform and shared codebase for both roles made the firmware easier to maintain while still allowing each build to specialize for its intended function.


<div align="center">
<img width="900"  src="/PCB/images/PCB.png">
</div>
