# CSEL - Mini Project

## Overview

The goal of this mini project is to implement a an application that can read CPU temperature and control its fan speed.
Since there is no fan on the NanoPi NeoPlus2, the led will simulate the fan speed: the higher the fan speed, the higher the LED blink frequency.

## Sysfs interface specification

### /sys/devices/platform/csel/mode

Allows to set the mode to auto or manual.
- **Write**: `auto` or `manual`
- **Read**: Current mode (either `auto` or `manual`)

### /sys/devices/platform/csel/temp

Allows to read the CPU temperature.
- **Write**: Not allowed
- **Read**: Current CPU temperature in degrees Celsius

### /sys/devices/platform/csel/blink_freq

Allows to set or read the LED blink frequency.

- **Write**: Frequency in Hz (e.g., `1`, `2`, `5`, etc.). If mode is `auto`, the write operation will not take effect.
- **Read**: Current blink frequency in Hz

> Min frequency is 1 Hz, max frequency is 20 Hz. If any other value is written, it will be clamped to the range [1, 20].

