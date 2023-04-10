// from https://github.com/gdsports/IntelliKeys_uhls

#ifndef _INTELLIKEYSDEFS_H_
#define _INTELLIKEYSDEFS_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

enum IK_LEDS {
  IK_LED_SHIFT = 1,
  IK_LED_CAPS_LOCK = 4,
  IK_LED_MOUSE = 7,
  IK_LED_ALT = 2,
  IK_LED_CTRL_CMD = 5,
  IK_LED_NUM_LOCK = 8
};

/*
 * Most of this file is extracted from the OpenIKeys project.
 */

#define IK_VID 0x095e
#define IK_PID_FWLOAD 0x0100  // Firmware load required
#define IK_PID_RUNNING 0x0101 // Firmware running

#define IK_REPORT_LEN 8

//  resolution of the device
#define IK_RESOLUTION_X 24
#define IK_RESOLUTION_Y 24

//  number of switches
#define IK_NUM_SWITCHES 6

//  number of sensors
#define IK_NUM_SENSORS 3

//
//  command codes sent to the device
//  see firmware documentation for details
//

#define CMD_BASE 0
#define IK_CMD_GET_VERSION (CMD_BASE + 1)
#define IK_CMD_LED (CMD_BASE + 2)
#define IK_CMD_SCAN (CMD_BASE + 3)
#define IK_CMD_TONE (CMD_BASE + 4)
#define IK_CMD_GET_EVENT (CMD_BASE + 5)
#define IK_CMD_INIT (CMD_BASE + 6)
#define IK_CMD_EEPROM_READ (CMD_BASE + 7)
#define IK_CMD_EEPROM_WRITE (CMD_BASE + 8)
#define IK_CMD_ONOFFSWITCH (CMD_BASE + 9)
#define IK_CMD_CORRECT (CMD_BASE + 10)
#define IK_CMD_EEPROM_READBYTE (CMD_BASE + 11)
#define IK_CMD_RESET_DEVICE (CMD_BASE + 12)
#define IK_CMD_START_AUTO (CMD_BASE + 13)
#define IK_CMD_STOP_AUTO (CMD_BASE + 14)
#define IK_CMD_ALL_LEDS (CMD_BASE + 15)
#define IK_CMD_START_OUTPUT (CMD_BASE + 16)
#define IK_CMD_STOP_OUTPUT (CMD_BASE + 17)
#define IK_CMD_ALL_SENSORS (CMD_BASE + 18)

#define JWH 1

#if JWH
#define IK_CMD_REFLECT_KEYSTROKE                                               \
  (CMD_BASE + 21) // send keystroke to host via ezusb
#define IK_CMD_REFLECT_MOUSE_MOVE                                              \
  (CMD_BASE + 22) // send mouse move to host via ezusb
#endif

#define IK_CMD_GET_SN CMD_BASE + 40

//  some internal commands that don't make it to the device
#define COMMAND_BASE 100                       // 0x64
#define IK_CMD_DELAY (COMMAND_BASE + 1)        // (msec)
#define IK_CMD_MOUSE_MOVE (COMMAND_BASE + 2)   // (x, y)
#define IK_CMD_MOUSE_BUTTON (COMMAND_BASE + 3) // (left/right, down/up)
#define IK_CMD_KEYBOARD (COMMAND_BASE + 4)     // (keycode, down/up)
#define IK_CMD_KEY_DONE                                                        \
  (COMMAND_BASE + 5) // signal downstream that a key is done
#define IK_CMD_KEY_START                                                       \
  (COMMAND_BASE + 6) // signal downstream that a key is starting
#define IK_CMD_KEY_REPEAT                                                      \
  (COMMAND_BASE + 7) // signal downstream that a key is repeating

#define IK_CMD_CP_HELP (COMMAND_BASE + 8)
#define IK_CMD_CP_LIST_FEATURES (COMMAND_BASE + 9)
#define IK_CMD_CP_REFRESH (COMMAND_BASE + 10)
#define IK_CMD_CP_TOGGLE (COMMAND_BASE + 11)

#define IK_CMD_KEYBOARD_UNICODE (COMMAND_BASE + 12) // lead, trail, down/up
#define IK_CMD_LIFTALLMODIFIERS (COMMAND_BASE + 13)
#define IK_CMD_CP_REPORT_REALTIME (COMMAND_BASE + 14)

//
//  result codes/data sent to the software
//  see firmware documentation for details
//
#define EVENT_BASE 50 // 0x32
#define IK_EVENT_ACK (EVENT_BASE + 1)
#define IK_EVENT_MEMBRANE_PRESS (EVENT_BASE + 2)
#define IK_EVENT_MEMBRANE_RELEASE (EVENT_BASE + 3)
#define IK_EVENT_SWITCH (EVENT_BASE + 4)
#define IK_EVENT_SENSOR_CHANGE (EVENT_BASE + 5)
#define IK_EVENT_VERSION (EVENT_BASE + 6)
#define IK_EVENT_EEPROM_READ (EVENT_BASE + 7)
#define IK_EVENT_ONOFFSWITCH (EVENT_BASE + 8)
#define IK_EVENT_NOMOREEVENTS (EVENT_BASE + 9)
#define IK_EVENT_MEMBRANE_REPEAT (EVENT_BASE + 10)
#define IK_EVENT_SWITCH_REPEAT (EVENT_BASE + 11)
#define IK_EVENT_CORRECT_MEMBRANE (EVENT_BASE + 12)
#define IK_EVENT_CORRECT_SWITCH (EVENT_BASE + 13)
#define IK_EVENT_CORRECT_DONE (EVENT_BASE + 14)
#define IK_EVENT_EEPROM_READBYTE (EVENT_BASE + 15)
#define IK_EVENT_DEVICEREADY (EVENT_BASE + 16)
#define IK_EVENT_AUTOPILOT_STATE (EVENT_BASE + 17)
#define IK_EVENT_DELAY (EVENT_BASE + 18)
#define IK_EVENT_ALL_SENSORS (EVENT_BASE + 19)

#define IK_FIRSTUNUSED_EVENTCODE EVENT_BASE + 20

// Arduino IK driver events
#define AIK_EVENT_BASE 80
#define IK_EVENT_CONNECT AIK_EVENT_BASE + 1
#define IK_EVENT_DISCONNECT AIK_EVENT_BASE + 2
#define IK_EVENT_SERNUM AIK_EVENT_BASE + 3

//
//  number of light sensors for reading overlay bar codes
//
#define IK_MAX_SENSORS 3

//
//  event reporting mode
//    auto   = data sent to software without specific command
//    polled = data sent to software only when asked
//
#define IK_EVENT_MODE_AUTO 0
#define IK_EVENT_MODE_POLLED 1

//
// Vendor specific request code for Anchor Upload/Download
//
// This one is implemented in the core
//
#define ANCHOR_LOAD_INTERNAL 0xA0
#define ANCHOR_LOAD_EXTERNAL 0xA3

//
// This is the highest internal RAM address for the AN2131Q
//
#define MAX_INTERNAL_ADDRESS 0x1B3F

#define INTERNAL_RAM(address) ((address <= MAX_INTERNAL_ADDRESS) ? true : false)

//
// EZ-USB Control and Status Register.  Bit 0 controls 8051 reset
//
#define CPUCS_REG 0x7F92

//--------------------------------------------------------------------+
//
//--------------------------------------------------------------------+

#define MAX_INTEL_HEX_RECORD_LENGTH 16

typedef struct _INTEL_HEX_RECORD {
  uint8_t Length;
  uint16_t Address;
  uint8_t Type;
  uint8_t Data[MAX_INTEL_HEX_RECORD_LENGTH];
} INTEL_HEX_RECORD, *PINTEL_HEX_RECORD;

#define IK_EEPROM_SN_SIZE 29

typedef struct {
  uint8_t serialnumber[IK_EEPROM_SN_SIZE];
  uint8_t sensorBlack[IK_NUM_SENSORS];
  uint8_t sensorWhite[IK_NUM_SENSORS];
} __attribute__((packed)) eeprom_t;

#endif /* _INTELLIKEYSDEFS_H_ */
