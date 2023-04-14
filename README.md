# Adafruit IntelliKeys

Adafruit IntelliKeys USB Translator uses both TinyUSB device and host stacks on 2 USB ports of rp2040 where:

- Native USB is running as device
- PICO-PIO-USB is running as host

USB host will enumerate IntelliKeys, download ez-usb firmware and then operate as host driver including scanning overlay sensor, membrane matrix and switch, set LEDs and buzzer etc... then translate corresponding key events to standard USB keyboard/mouse to the PC host via device interface.

## Supported Hardware

This library supports the following hardware:

- [Adafruit Feather RP2040 with USB Type A Host](https://www.adafruit.com/product/5723)

## Features

TODO

- control/command/atl latching
- mouse down: released by mouse down, left, double click
- shift key latching/non-latching/locking
- Multiple keystrokes like www. and .com
- Mouse double clicks
- Support setup overlays for behavior settings (volume, repeat rate, mouse speed). Maybe stores settings in text file (exposed by MSC)
- Custom overlays in text file


## Installation

- Put board to DFU mode by pressing BOOTSEL button and then reset button
- Download UF2 from release page and copy to the board

## References

- https://github.com/ATMakersOrg/OpenIKeys
- https://github.com/gdsports/IntelliKeys_uhls
