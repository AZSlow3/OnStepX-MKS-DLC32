OnStepX for MKS DLC32 shield
===========================
A modification of OnStepX to support MKS-DLC32 shield v2 (https://github.com/makerbase-mks/MKS-DLC32)
WARNING: see "Current status" before using.

To use this fork you still should follow original instructions (https://onstep.groups.io/g/main/wiki/32776).
Drivers, mount, BT, WiFi, etc. are particular setup specific and should be explicitly specified.

For this board PINMAP must be MKS_DLC32_V2 (it will set GPIO_DEVICE).
In the pinmap, the parameters of the third driver are defined for AXIS3 or AXIS4, depending
from AXIS3/4_DRIVER_MODEL. So you can use it for rotator or for focuser.

Display driver is LovyanGFX. In my tests its touch implementation is way faster (TFT_eSPI use many delay()
in getting current status) and the library allows local config (my dev setup has different display).
Display code in in MKS_TS35R plug-in.

Note that controlling drivers with UART is not possible with this board. So GENERIC driver model is the only option.

See comments in Pins.MKS-DLC32.h for other possible options and features.

Note: website plug-in should be manually copied.

Current status:
Tested on MKS DLC32 with 3x MKS TMC2209 v2.9 drivers (64 micro-steps per step mode) and 3 NEMA 17 1.5A 1.8° per step motors.
Everything works as expected, but I have not properly alligned yet. So I had no "perfect GoTo" results.

OnStepX Telescope Controller
===========================
https://github.com/hjd1964/OnStepX
