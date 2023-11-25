// -------------------------------------------------------------------------------------------------
// Pin map for MKS DLC32 v2 (do NOT use with v1, it has different pinning, f.e. motor<->probe)
#pragma once

#if defined(ESP32)

// In SCH for MKS DLC32, ESP pins are numbered and named. Numbers are not matching any other common documents,
// so the should not be used. IOxx are normally referenced as GPIOxx and used in C programs (including OnStep)
// some named pins don't mention IO, so here is the map for them (not really checked!)
// SENSOR_VP IO36
// SENSOR_VN IO39
// RXD0      IO03
// TXD0      IO01
// Originl pins definition is at the beginning of i2s_out_xyz_mks_dlc32.h Machines

// Usable features of MKS DLC32 v2 board:
//  In short:
//    Up to 3 drivers can be controlled by DIR/STEP method (no UART). Dedicated Reset (Emergency Stop) button socket,
//    4 sockets for other buttons or sensors (to the ground), I2C shields (some can be incompatible), MKS T35 touch display,
//    WiFi, BT, USB, SD-Card reader.
//
//  Complete list (epart from standard ESP32 features like WiFi and BT):
//  * up to 3 step/dir drivers with common enable are controllable throw GPIO (shift register), IO (16,17,21). 
//    Pre-defined in this file.
//  * Beeper output throw GPIO (so sigitat) is available on display socket and on dedicated socket (12V !)
//    Not (yet) defined.
//  * There are many pins dedicated to TFT LCD touch display (TS35/TS24).
//    LCD_CS_0 (IO25), LCD_TOUCH_CS_0 (IO26), LCD_RST_0 (IO27), LCD_EN_0 (IO5) are connected to U7 and U8 (??? what are they),
//    while LCD_SCK (IO18), LCD_MISO (IO19), LCD_MOSI (IO23) are directly available on display socket and so
//    potentiall can be used for other perpose (if display is not used).
//    I (plan to) use the display.
//  * there is SD card reader, connected to IO (39,14,12,13,15). Note in the original firmware is is not used for writing.
//    Not (yet) defined
//  * there is I2C bus on (unusual!) IO (0,4), there is dedicated socket to connect related defined (RTC, Weather).
//    There is USB control for IO0 (and IO2), since they are used to reboot and set flush mode (influence enabel/reset pin).
//    So connected device (theoretically) can prevent boot/flash if they set "bad level" (which?) initially on SDA (IO0).
//    TODO: check that common shilds (DS3231, BMP280) are "compatible".
//    I2C itself is defined in this file (connected devices should be configured by user).
//  * there is a socket for "Reset" button
//  * Serial connection (the only) is routed to USB (CH340). There are LEDs to indicate RX/TX there.
//    Configured per default.
//  * there is a socket for power switch, which can be used instead of fuse (it is better to have a fuse in the switch then...) 
//    Note there is another (soldered) fuse for low voltage (before 5V converter). There are 3 LEDs to indicate all voltages
//    are present.
//  * the board is designed to control laser or CNC DC motor with IO32. It is converted to TTL (+5V) level to be
//    used as TTL PWM output for 3pins PWM laser (dedicated socket), duplicated on 12V (+12V always connected) socket
//    for 2pins PWM laser or DC motor.
//  * there are 4 pins for X, Y, Z limit sensors and Probe, IO (36,35,34,22).
//    They are 2R+C protected (Probe 2 diodes protected) and pulled up (normal high). Buttons to the ground can be directly
//    attached to the corresponding dedicated sockets (Warning: the third pin on these sockets is +5V! do NOT connect buttons to it)
//    TODO: make example definitions, test with firmware
//  * there is an LED on IO2, but since that is "special" pin and LED is on-board, it is a bad target for custom manipulations.

#if SERIAL_A_BAUD_DEFAULT != OFF
  #define SERIAL_A              Serial
#endif

// Can't use TMC UART (for TMC2209 & Co) on this board, no SERIAL_TMC

// These are NOT usual ESP32 I2C pins (21,22), 21 is used for 74HC595 data, 22 for probe
// Note: on my 30pin dev board IO0 is not exposed, I will use 22
#if defined(MKS_DLC32_DEV)
  #define IC2_SDA_PIN            22
#else
  #define I2C_SDA_PIN             0
#endif
#define I2C_SCL_PIN             4

// TODO: I guess there are possible AUX definitions for this board...

// Drivers are connected to GPIO (8bit shift register)
#ifdef GPIO_DEVICE
  #undef GRIO_DEVICE
#endif
#define GPIO_DEVICE SSR74HC595
#define GPIO_SSR74HC595_LATCH_PIN  17
#define GPIO_SSR74HC595_CLOCK_PIN  16
#define GPIO_SSR74HC595_DATA_PIN   21
#define GPIO_SSR74HC595_COUNT       8

#define SHARED_ENABLE_PIN   GPIO_PIN(0)            // Hint that the direction pins are shared
#define SHARED_ENABLE_PIN      5                // Hint that the enable pins are shared

// ??? Should I define GPIO_DIRECTION_PINS ??? not used in any current definition...

// Axis1 RA/Azm step/dir driver
#define AXIS1_ENABLE_PIN        SHARED           // Enable pin control
#define AXIS1_M0_PIN            OFF
#define AXIS1_M1_PIN            OFF
#define AXIS1_M2_PIN            OFF
#define AXIS1_M3_PIN            OFF
#define AXIS1_STEP_PIN          GPIO_PIN(1)
#define AXIS1_DIR_PIN           GPIO_PIN(2)
// TODO: AXIS1_SENSE_HOME_PIN probably can be X limit sensor

// Axis2 Dec/Alt step/dir driver
#define AXIS2_ENABLE_PIN        SHARED           // Enable pin control
#define AXIS2_M0_PIN            OFF
#define AXIS2_M1_PIN            OFF
#define AXIS2_M2_PIN            OFF
#define AXIS2_M3_PIN            OFF
#define AXIS2_STEP_PIN          GPIO_PIN(5)
#define AXIS2_DIR_PIN           GPIO_PIN(6)
// TODO: AXIS2_SENSE_HOME_PIN probably can be Y limit sensor

#if defined(AXIS3_DRIVER_MODEL) && (AXIS3_DRIVER_MODEL != OFF)
  // Use for rotator
  #define AXIS3_ENABLE_PIN        SHARED           // Enable pin control
  #define AXIS3_M0_PIN            OFF
  #define AXIS3_M1_PIN            OFF
  #define AXIS3_M2_PIN            OFF
  #define AXIS3_M3_PIN            OFF
  #define AXIS3_STEP_PIN          GPIO_PIN(3)
  #define AXIS3_DIR_PIN           GPIO_PIN(4)
  // TODO: AXIS3_SENSE_HOME_PIN probably can be Z limit sensor
#elif defined(AXIS4_DRIVER_MODEL) && (AXIS4_DRIVER_MODEL != OFF)
  // Use for focused
  #define AXIS4_ENABLE_PIN        SHARED           // Enable pin control
  #define AXIS4_M0_PIN            OFF
  #define AXIS4_M1_PIN            OFF
  #define AXIS4_M2_PIN            OFF
  #define AXIS4_M3_PIN            OFF
  #define AXIS4_STEP_PIN          GPIO_PIN(3)
  #define AXIS4_DIR_PIN           GPIO_PIN(4)
#endif

#else
#error "Wrong processor for this configuration!"

#endif
