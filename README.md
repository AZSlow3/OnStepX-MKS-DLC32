OnStepX for MKS DLC32 shield
===========================
A modification of OnStepX to support MKS-DLC32 shield v2 (https://github.com/makerbase-mks/MKS-DLC32)
WARNING: not tested yet, do NOT use!!!

To use this fork you still should follow original instructions (https://onstep.groups.io/g/main/wiki/32776).
Drivers, mount, BT, WiFi, etc. are particular setup specific and should be explicitly specified.

For this board PINMAP must be MKS_DLC32_V2 (it will set GPIO_DEVICE).
In the pinmap, the parameters of the third driver are defined for AXIS3 or AXIS4, depending
from AXIS3/4_DRIVER_MODEL. So you can use it for rotator or for focuser.

Note that controlling drivers with UART is not possible with this board. So GENERIC driver model is the only option.

See comments in Pins.MKS-DLC32.h for other possible options and features.

Current status:
Tested on ESP-WROOM-32 "NodeMCU" 30pin board + dev board (to connect 12V power), with 74HC595D GPIO connected according to MKS-DLC32 documentation.
One NEMA 17 1.5A connected throw MKS TMC2209 v2.0 seems like reacting right on commands from Android App (was set to 8 micro-steps, in "GoTo"
was "stalled" once turning fast with default settings, I hope the limitation comes from the hardware, but I doubt I will even need such speed
with 64 micro-steps).

OnStepX Telescope Controller
===========================
https://github.com/hjd1964/OnStepX
