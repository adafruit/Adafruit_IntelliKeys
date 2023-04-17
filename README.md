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

- Download ez-usb firmware from USB host with color indicator. Red for no usb, yellow for downloading/not ready, green for ready.
- IntelliKeys overlay detection with LEDs and sound indicator
- Support all standard overlays (except setup) with both keyboard and mouse.
- Support all modifier latching for keys like shift, ctrl, alt, command/win/super
- Support toggle (on/off) switch detection (yellow LED)
- Support custom overlays but required re-compiled firmware with new overlay definition.

TODO (not supported yet):

- Switch is not supported due to lack of testing hardware
- Multiple keystrokes like www. and .com
- Mouse double clicks
- Support setup overlays for behavior settings (volume, repeat rate, mouse speed). Maybe stores settings in text file (exposed by MSC)
- Custom overlays in text file in MSC

## Installation

- Put board to DFU mode by pressing BOOTSEL button and then reset button
- Download UF2 from release page and copy to the board

## References

- https://github.com/ATMakersOrg/OpenIKeys
- https://github.com/gdsports/IntelliKeys_uhls
