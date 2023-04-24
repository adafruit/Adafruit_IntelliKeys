# Adafruit IntelliKeys

Adafruit IntelliKeys USB Translator uses both TinyUSB device and host stacks on 2 USB ports of rp2040 where:

- Native USB is running as device
- PICO-PIO-USB is running as host

USB host will enumerate IntelliKeys, download ez-usb firmware and then operate as host driver including scanning overlay sensor, membrane matrix and switch, set LEDs and buzzer etc... then translate corresponding key events to standard USB keyboard/mouse to the PC host via device interface.

## Supported Hardware

This library supports the following hardware:

- [Adafruit Feather RP2040 with USB Type A Host](https://www.adafruit.com/product/5723)

## Features

This library tries to support all features of the original IntelliKeys USB driver. The following features are supported:

- Download ez-usb firmware from USB host
- IntelliKeys overlay detection with LEDs and sound indicator
- Support all standard overlays (except setup) with both keyboard and mouse.
- Support all modifier latching for keys like shift, ctrl, alt, command/win/super
- Support toggle (on/off) switch detection (yellow LED)
- Support custom overlays but required re-compiled firmware with new overlay definition.

TODO (not supported yet):

- Switch is not supported due to lack of testing hardware
- Multiple reports event such as: keystrokes like www. and .com  and mouse double clicks
- Support setup overlays for behavior settings (volume, repeat rate, mouse speed). Maybe stores settings in text file (exposed by MSC)
- Custom overlays in text file in MSC

## Installation

- Put board to DFU mode by pressing BOOTSEL button and then reset button
- Download UF2 from release page and copy to the board

## Usage

- When powering on or without IKey device, neopixel will be red. Plugging IKey into USB host connector, neopixel will be yellow to indicate that IKey firmware is downloading and then green when ready. Note: it will briefly show red when device is re-enumerated.
- Toggle IKeys on/off switch will play beep sound and change neopixel from green (on) to yellow (off) or vice versa.
- After IKey firmware is ready, we will initialize IKey device and then start scanning photo sensors for overlay. If overlay changes is detected, we will play a long beep sound and flash device LEDs.
- If overlay is detected, we will scan membrane matrix and switch. If any key is pressed, there is a short beep sound as well as neopixel color set to blue (key pressed) or green (key released) for indicator.
- All membrane and switch changes will be accumulated using an 2-dimension array, which should be poll regularly (2-8ms) and then translated to standard USB keyboard/mouse events according to overlay data. If there is any change, we will send a report to PC.
- All modifier keys: Control, Shift, Alt/Option, Command/Windows/Super are latching key, which means they will retain their state until they are pressed again. IKeys LEDs will also bet set accordingly.
- Custom overlays are supported, however, it requires re-compiled firmware with new overlay definition. For how to define an overlay, check out `src/overlay.h` and `src/overlay.c` for details. All custom overlay number must start from 8 since 0-7 is reserved for standard overlays.

## References

- https://github.com/ATMakersOrg/OpenIKeys
- https://github.com/gdsports/IntelliKeys_uhls
