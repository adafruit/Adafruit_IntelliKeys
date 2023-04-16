/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Ha Thach (tinyusb.org) for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>

#include "Adafruit_IntelliKeys.h"
#include "intellikeysdefs.h"

#include "ik_firmware.h"
#include "ik_loader.h"

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

#define IK_DEBUG 1
#define IK_VID 0x095e
#define IK_PID_FWLOAD 0x0100  // Firmware load required
#define IK_PID_RUNNING 0x0101 // Firmware running

#if IK_DEBUG

#define IK_PRINTF(...) Serial.printf(__VA_ARGS__)

const char *const ik_cmd_str[] = {
    [CMD_BASE] = "BASE",
    [IK_CMD_GET_VERSION] = "GET_VERSION",
    [IK_CMD_LED] = "LED",
    [IK_CMD_SCAN] = "SCAN",
    [IK_CMD_TONE] = "TONE",
    [IK_CMD_GET_EVENT] = "GET_EVENT",
    [IK_CMD_INIT] = "INIT",
    [IK_CMD_EEPROM_READ] = "EEPROM_READ",
    [IK_CMD_EEPROM_WRITE] = "EEPROM_WRITE",
    [IK_CMD_ONOFFSWITCH] = "ONOFFSWITCH",
    [IK_CMD_CORRECT] = "CORRECT",
    [IK_CMD_EEPROM_READBYTE] = "EEPROM_READBYTE",
    [IK_CMD_RESET_DEVICE] = "RESET_DEVICE",
    [IK_CMD_START_AUTO] = "START_AUTO",
    [IK_CMD_STOP_AUTO] = "STOP_AUTO",
    [IK_CMD_ALL_LEDS] = "ALL_LEDS",
    [IK_CMD_START_OUTPUT] = "START_OUTPUT",
    [IK_CMD_STOP_OUTPUT] = "STOP_OUTPUT",
    [IK_CMD_ALL_SENSORS] = "ALL_SENSORS",
    [CMD_BASE + 19] = "UNKNOWN",
    [CMD_BASE + 20] = "UNKNOWN",
    [IK_CMD_REFLECT_KEYSTROKE] = "REFLECT_KEYSTROKE",
    [IK_CMD_REFLECT_MOUSE_MOVE] = "REFLECT_MOUSE_MOVE",
};

const char *const ik_cmd_local_str[]{
    [COMMAND_BASE - COMMAND_BASE] = "BASE",
    [IK_CMD_DELAY - COMMAND_BASE] = "IK_CMD_DELAY",
    [IK_CMD_MOUSE_MOVE - COMMAND_BASE] = "IK_CMD_MOUSE_MOVE",
    [IK_CMD_MOUSE_BUTTON - COMMAND_BASE] = "IK_CMD_MOUSE_BUTTON",
    [IK_CMD_KEYBOARD - COMMAND_BASE] = "IK_CMD_KEYBOARD",
    [IK_CMD_KEY_DONE - COMMAND_BASE] = "IK_CMD_KEY_DONE",
    [IK_CMD_KEY_START - COMMAND_BASE] = "IK_CMD_KEY_START",
    [IK_CMD_KEY_REPEAT - COMMAND_BASE] = "IK_CMD_KEY_REPEAT",
    [IK_CMD_CP_HELP - COMMAND_BASE] = "IK_CMD_CP_HELP",
    [IK_CMD_CP_LIST_FEATURES - COMMAND_BASE] = "IK_CMD_CP_LIST_FEATURES",
    [IK_CMD_CP_REFRESH - COMMAND_BASE] = "IK_CMD_CP_REFRESH",
    [IK_CMD_CP_TOGGLE - COMMAND_BASE] = "IK_CMD_CP_TOGGLE",
    [IK_CMD_KEYBOARD_UNICODE - COMMAND_BASE] = "IK_CMD_KEYBOARD_UNICODE",
    [IK_CMD_LIFTALLMODIFIERS - COMMAND_BASE] = "IK_CMD_LIFTALLMODIFIERS",
    [IK_CMD_CP_REPORT_REALTIME - COMMAND_BASE] = "IK_CMD_CP_REPORT_REALTIME",
};

const char *const ik_event_str[] = {
    [EVENT_BASE - EVENT_BASE] = "BASE",
    [IK_EVENT_ACK - EVENT_BASE] = "ACK",
    [IK_EVENT_MEMBRANE_PRESS - EVENT_BASE] = "MEMBRANE_PRESS",
    [IK_EVENT_MEMBRANE_RELEASE - EVENT_BASE] = "MEMBRANE_RELEASE",
    [IK_EVENT_SWITCH - EVENT_BASE] = "SWITCH",
    [IK_EVENT_SENSOR_CHANGE - EVENT_BASE] = "SENSOR_CHANGE",
    [IK_EVENT_VERSION - EVENT_BASE] = "VERSION",
    [IK_EVENT_EEPROM_READ - EVENT_BASE] = "EEPROM_READ",
    [IK_EVENT_ONOFFSWITCH - EVENT_BASE] = "ONOFFSWITCH",
    [IK_EVENT_NOMOREEVENTS - EVENT_BASE] = "NOMOREEVENTS",
    [IK_EVENT_MEMBRANE_REPEAT - EVENT_BASE] = "MEMBRANE_REPEAT",
    [IK_EVENT_SWITCH_REPEAT - EVENT_BASE] = "SWITCH_REPEAT",
    [IK_EVENT_CORRECT_MEMBRANE - EVENT_BASE] = "CORRECT_MEMBRANE",
    [IK_EVENT_CORRECT_SWITCH - EVENT_BASE] = "CORRECT_SWITCH",
    [IK_EVENT_CORRECT_DONE - EVENT_BASE] = "CORRECT_DONE",
    [IK_EVENT_EEPROM_READBYTE - EVENT_BASE] = "EEPROM_READBYTE",
    [IK_EVENT_DEVICEREADY - EVENT_BASE] = "DEVICEREADY",
    [IK_EVENT_AUTOPILOT_STATE - EVENT_BASE] = "AUTOPILOT_STATE",
    [IK_EVENT_DELAY - EVENT_BASE] = "DELAY",
    [IK_EVENT_ALL_SENSORS - EVENT_BASE] = "ALL_SENSORS",
};

#else
#define IK_PRINTF(...)
#endif

#if IK_DEBUG >= 2
#define IK_PRINTF2 IK_PRINTF
#else
#define IK_PRINTF2(...)
#endif

//--------------------------------------------------------------------+
// Public API
//--------------------------------------------------------------------+

Adafruit_IntelliKeys::Adafruit_IntelliKeys(void) {
  Reset();

  _membrane_cb = NULL;
  _switch_cb = NULL;
  _toggle_cb = NULL;

  _custom_overlay = NULL;
  _custom_overlay_count = 0;

  tu_fifo_config(&_cmd_ff, _cmd_ff_buf, IK_CMD_FIFO_SIZE, 8, false);
  tu_fifo_config_mutex(&_cmd_ff, osal_mutex_create(&_cmd_ff_mutex), NULL);

  //
}

void Adafruit_IntelliKeys::Reset(void) {
  _daddr = 0;
  _opened = false;

  m_lastLEDTime = 0;
  m_delayUntil = 0;
  m_nextCorrect = 0;

  m_newLevel = 0;
  m_currentLevel = 0;

  m_lastOverlay = -1;
  m_lastOverlayTime = 0;
  m_currentOverlay = -1;

  m_toggle = -1;

  memset(m_membrane, 0, sizeof(m_membrane));
  memset(m_last_membrane, 0, sizeof(m_last_membrane));
  memset(m_switches, 0, sizeof(m_switches));

  m_bEepromValid = false;

  m_firmwareVersionMajor = 0;
  m_firmwareVersionMinor = 0;

  m_lastSwitch = 0;
}

void Adafruit_IntelliKeys::begin(void) { IKOverlay::initStandardOverlays(); }

bool Adafruit_IntelliKeys::mount(uint8_t daddr) {
  uint16_t vid, pid;
  tuh_vid_pid_get(daddr, &vid, &pid);

  Serial.printf("VID = %04x, PID = %04x\r\n", vid, pid);

  if (vid != IK_VID) {
    return false;
  }

  _daddr = daddr;

  if (pid == IK_PID_FWLOAD) {
    IK_PRINTF("IK mounted without firmware\n");
    ezusb_StartDevice();
  } else if (pid == IK_PID_RUNNING) {
    IK_PRINTF("IK mounted running firmware\n");

    if (!tuh_hid_receive_report(_daddr, 0)) {
      IK_PRINTF("Failed to receive report\n");
      return false;
    }

    // Start IK device
    Start();
  } else {
    return false;
  }

  return true;
}

void Adafruit_IntelliKeys::umount(uint8_t daddr) {
  if (daddr == _daddr) {
    Reset();
  }
}

void Adafruit_IntelliKeys::Periodic(void) {
  if (!IsOpen()) {
    return; // nothing to do
  }

  // settle overlay
  SettleOverlay();

  uint32_t now = millis();

  //  setLEDs
  if (now > m_lastLEDTime + 100) {
    SetLEDs();
    m_lastLEDTime = now;
  }

  //  request a correction every so often.
  if (now > m_nextCorrect) {
    DoCorrect();
    m_nextCorrect = now + 500;
  }

  //  send for not-yet valid eeprom bytes
  if (!m_bEepromValid) {
    for (uint8_t i = 0; i < sizeof(eeprom_t); i++) {
      if (!m_eepromDataValid[i] && m_eepromRequestTime[i] + 500 < now) {
        uint8_t report[8] = {
            IK_CMD_EEPROM_READBYTE, (uint8_t)(0x80 + i), 0x1F, 0, 0, 0, 0, 0};
        PostCommand(report);
        m_eepromRequestTime[i] = now;
      }
    }
  }

  ProcessCommands();

  // InterpretRaw();
}

static bool checkNewKeyboardReport(hid_keyboard_report_t const *report,
                                   ik_report_keyboard_t *ik_keyboard) {
  if (ik_keyboard->modifier != 0 &&
      !(report->modifier & ik_keyboard->modifier)) {
    return true;
  }

  for (uint8_t i = 0; i < 6; i++) {
    if (ik_keyboard->keycode == report->keycode[i]) {
      return false;
    }
  }

  return true;
}

static void combineMouseReport(hid_mouse_report_t *report,
                               ik_report_mouse_t *ik_mouse) {
  report->buttons |= ik_mouse->buttons;
  report->x += ik_mouse->x;
  report->y += ik_mouse->y;
}

void Adafruit_IntelliKeys::getHIDReport(hid_keyboard_report_t *kb_report,
                                        hid_mouse_report_t *mouse_report) {
  memset(kb_report, 0, sizeof(hid_keyboard_report_t));
  memset(mouse_report, 0, sizeof(hid_mouse_report_t));

  if (!IsOpen() || !IsSwitchedOn()) {
    return;
  }

  IKOverlay *overlay = GetCurrentOverlay();
  if (overlay == NULL) {
    return;
  }

  uint8_t kb_count = 0;

  //------------- scan membrane -------------//
  for (uint8_t i = 0; i < IK_RESOLUTION_X; i++) {
    for (uint8_t j = 0; j < IK_RESOLUTION_Y; j++) {
      if (m_membrane[i][j] == 1) {
        ik_report_t ik_report;
        overlay->getMembraneReport(i, j, &ik_report);

        if (ik_report.type == IK_REPORT_TYPE_KEYBOARD) {
          // Serial.printf(
          //    "rol = %u, col = %u, modifier = %02X, keycode = %02X\r\n", i, j,
          //    ik_report.keyboard.modifier, ik_report.keyboard.keycode);
          if (checkNewKeyboardReport(kb_report, &ik_report.keyboard)) {
            kb_report->modifier |= ik_report.keyboard.modifier;
            if (ik_report.keyboard.keycode != 0) {
              kb_report->keycode[kb_count] = ik_report.keyboard.keycode;
              kb_count++;
              if (kb_count >= 6) {
                break;
              }
            }
          }
        } else if (ik_report.type == IK_REPORT_TYPE_MOUSE) {
          //          Serial.printf(
          //              "rol = %u, col = %u, buttons = %02X, x = %d, y =
          //              %d\r\n", i, j, ik_report.mouse.buttons,
          //              ik_report.mouse.x, ik_report.mouse.y);
          combineMouseReport(mouse_report, &ik_report.mouse);
        }
      }
    }
  }

  // Check for modifier latching

  // TODO scan switch
}

void Adafruit_IntelliKeys::InterpretRaw() {
  //  don't bother if we're not connected and switched on
  if (!IsOpen()) {
    return;
  }
  if (!IsSwitchedOn()) {
    return;
  }

  IKOverlay *overlay = GetCurrentOverlay();

  //  look for _membrane change
  for (uint8_t col = 0; col < IK_RESOLUTION_X; col++) {
    for (uint8_t row = 0; row < IK_RESOLUTION_Y; row++) {
      const uint8_t state = m_membrane[row][col];
      if (state != m_last_membrane[row][col]) {
        IK_PRINTF("membrane [%02u, %02u] = %u\r\n", row, col, state);

        if (state) {
          ShortKeySound();

          // Modifier Latching
          if (overlay) {
            ik_report_t ik_report;
            overlay->getMembraneReport(row, col, &ik_report);
            if (ik_report.type == IK_REPORT_TYPE_KEYBOARD &&
                ik_report.keyboard.modifier) {
              uint8_t const modifier = ik_report.keyboard.modifier;

              if (modifier &
                  (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL)) {
                m_modControl.ToggleState();
              }

              if (modifier & (KEYBOARD_MODIFIER_LEFTSHIFT |
                              KEYBOARD_MODIFIER_RIGHTSHIFT)) {
                m_modShift.ToggleState();
              }

              if (modifier &
                  (KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_RIGHTALT)) {
                m_modAlt.ToggleState();
              }

              if (modifier &
                  (KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_RIGHTGUI)) {
                m_modCommand.ToggleState();
              }
            } else if (ik_report.type == IK_REPORT_TYPE_MOUSE) {
              if (ik_report.mouse.buttons & IK_REPORT_MOUSE_CLICK_HOLD) {
                m_mouseDown.ToggleState();
              }

              if (ik_report.mouse.buttons &
                  (MOUSE_BUTTON_LEFT | IK_REPORT_MOUSE_DOUBLE_CLICK)) {
                m_mouseDown.SetState(kModifierStateOff);
              }
            }
          }
        }

        // save current state for next time
        m_last_membrane[row][col] = state;

        if (_membrane_cb) {
          _membrane_cb(row, col, state);
        }
      }
    }
  }

  //  look for switch change
  for (uint8_t nsw = 0; nsw < IK_NUM_SWITCHES; nsw++) {
    if (m_switches[nsw] != m_last_switches[nsw]) {
      IK_PRINTF("switch %02u = %u\r\n", nsw, m_switches[nsw]);
      if (m_switches[nsw]) {
        ShortKeySound();
      }

      if (_switch_cb) {
        _switch_cb(nsw, m_switches[nsw]);
      }
      // save current state for next time
      m_last_switches[nsw] = m_switches[nsw];
    }
  }
}

//--------------------------------------------------------------------+
// Private
//--------------------------------------------------------------------+

bool Adafruit_IntelliKeys::Start(void) {
  uint8_t command[IK_REPORT_LEN] = {0};

  command[0] = IK_CMD_INIT;
  command[1] = 0; //  interrupt event mode
  PostCommand(command);

  command[0] = IK_CMD_SCAN;
  command[1] = 1; //  enable
  PostCommand(command);

  PostDelay(250);

  command[0] = IK_CMD_ALL_SENSORS;
  command[1] = 0; //  unused
  PostCommand(command);

  // ask the device for its firmware version
  command[0] = IK_CMD_GET_VERSION;
  command[1] = 0; //  unused
  PostCommand(command);

  //  reset keyboard
  ResetKeyboard();

  //  reset mouse
  ResetMouse();

  _opened = true;

  return false;
}

bool Adafruit_IntelliKeys::IsNumLockOn(void) {
  // implement later
  return false;
}

bool Adafruit_IntelliKeys::IsCapsLockOn(void) {
  // implement later
  return false;
}

bool Adafruit_IntelliKeys::IsMouseDown(void) {
  return m_mouseDown.GetState() != 0;
}

void Adafruit_IntelliKeys::DoCorrect(void) {
  //  clear out data
  for (int i = 0; i < IK_NUM_SWITCHES; i++) {
    m_switchesPressedInCorrectMode[i] = 0;
  }

  for (int x = 0; x < IK_RESOLUTION_X; x++) {
    for (int y = 0; y < IK_RESOLUTION_Y; y++) {
      m_membranePressedInCorrectMode[y][x] = 0;
    }
  }

  //  send the command
  uint8_t report[IK_REPORT_LEN] = {IK_CMD_CORRECT, 0, 0, 0, 0, 0, 0, 0};
  PostCommand(report);
}

void Adafruit_IntelliKeys::OnCorrectMembrane(int x, int y) {
  m_membranePressedInCorrectMode[y][x] = true;
}

void Adafruit_IntelliKeys::OnCorrectSwitch(int switchnum) {
  int ns = switchnum;
  m_switchesPressedInCorrectMode[ns - 1] = true;
}

void Adafruit_IntelliKeys::OnCorrectDone() {
  for (int i = 0; i < IK_NUM_SWITCHES; i++) {
    m_switches[i] = m_switchesPressedInCorrectMode[i];
  }

  for (int x = 0; x < IK_RESOLUTION_X; x++) {
    for (int y = 0; y < IK_RESOLUTION_Y; y++) {
      m_membrane[y][x] = m_membranePressedInCorrectMode[y][x];
    }
  }
}

void Adafruit_IntelliKeys::OnMembranePress(int x, int y) {
  m_membrane[y][x] = 1;
}

void Adafruit_IntelliKeys::OnMembraneRelease(int x, int y) {
  m_membrane[y][x] = 0;
}

// All commands processed in this function is sent to device
void Adafruit_IntelliKeys::ProcessCommands() {
  uint8_t const idx = 0;

  //  come back later if IK_ICMD_DELAY has set a future time
  if (millis() < m_delayUntil) {
    return;
  }

  if (!tuh_hid_send_ready(_daddr, idx)) {
    return;
  }

  uint8_t command[IK_REPORT_LEN] = {0};

  if (!tu_fifo_read(&_cmd_ff, command)) {
    return;
  }

  uint8_t const cmd_id = command[0];

  if (cmd_id < COMMAND_BASE) {
    if (cmd_id > IK_CMD_REFLECT_MOUSE_MOVE) {
      IK_PRINTF("ProcessCommand: invalid cmd %d\r\n", cmd_id);
    } else {
      IK_PRINTF2("ProcessCommand: %s\r\n", ik_cmd_str[cmd_id]);
    }

    // blocking until report is sent
    while (!tuh_hid_send_report(_daddr, idx, 0, command, IK_REPORT_LEN)) {
      tuh_task();
    }
  }
}

bool Adafruit_IntelliKeys::PostCommand(uint8_t *command) {
  uint8_t const cmd_id = command[0];

  if (cmd_id < COMMAND_BASE) {
    if (cmd_id > IK_CMD_REFLECT_MOUSE_MOVE) {
      IK_PRINTF("PostCommand: invalid cmd %d\r\n", cmd_id);
      return false;
    }
    // IK_PRINTF("PostCommand: %s\r\n", ik_cmd_str[cmd_id]);

    // queue command sent to device
    if (!tu_fifo_write(&_cmd_ff, command)) {
      IK_PRINTF("PostCommand: Failed to queue command, probably full. Please "
                "increase IK_CMD_FIFO_SIZE\n");
    }
  } else {
    // local driver command
    if (cmd_id > IK_CMD_CP_REPORT_REALTIME) {
      IK_PRINTF("PostCommand (local): invalid cmd %d\r\n", cmd_id);
      return false;
    } else {
      IK_PRINTF("PostCommand (local): %s\r\n",
                ik_cmd_local_str[cmd_id - COMMAND_BASE]);

      switch (cmd_id) {
      case IK_CMD_DELAY:
        m_delayUntil = millis() + command[1];
        break;

      default:
        break;
      }
    }
  }

  return true;
}

void Adafruit_IntelliKeys::PostSetLED(uint8_t number, uint8_t value) {
  uint8_t command[IK_REPORT_LEN] = {IK_CMD_LED, number, value, 0, 0, 0, 0, 0};
  PostCommand(command);
}

void Adafruit_IntelliKeys::PostDelay(uint8_t msec) {
  uint8_t command[IK_REPORT_LEN];
  command[0] = IK_CMD_DELAY;
  command[1] = msec; //  msec delay
  PostCommand(command);
}

void Adafruit_IntelliKeys::PostKey(int code, int direction, int delayAfter) {
  //  track shift status and last code up for smart typing.

  if (direction == IK_UP) {
    if (code == UNIVERSAL_SHIFT || code == UNIVERSAL_RIGHT_SHIFT) {
      m_bShifted = true;
    } else {
      m_lastCodeUp = code;
      m_bShifted = false;
    }
  }

  uint8_t command[IK_REPORT_LEN];
  command[0] = IK_CMD_KEYBOARD;
  command[1] = code;
  command[2] = direction;
  command[3] = delayAfter & 0xff;
  command[4] = (delayAfter / 256) & 0xff;
  PostCommand(command);
}

void Adafruit_IntelliKeys::SetLEDs(void) {
  if (!IsSwitchedOn()) {
    return;
  }

  if (!IsOpen()) {
    return;
  }

  bool bShift = (m_modShift.GetState() != 0);
  bool bControl = (m_modControl.GetState() != 0);
  bool bAlt = (m_modAlt.GetState() != 0);
  bool bCommand = (m_modCommand.GetState() != 0);
  bool bNumLock = IsNumLockOn();
  bool bMouse = IsMouseDown();
  bool bCapsLock = IsCapsLockOn();

  //  3 lights is shift, caps lock, mouse down
  PostSetLED(1, bShift);
  PostSetLED(4, bCapsLock);
  PostSetLED(7, bMouse);

  //  6 lights is alt, control/command, num lock
  bool b6lights =
      (IKSettings::GetSettings()->m_iIndicatorLights == kSettings6lights);
  if (b6lights) {
    PostSetLED(2, bAlt);
    PostSetLED(5, bControl || bCommand);
    PostSetLED(8, bNumLock);

    PostSetLED(3, false);
    PostSetLED(6, false);
    PostSetLED(9, false);
  } else {
    PostSetLED(3, bShift);
    PostSetLED(6, bCapsLock);
    PostSetLED(9, bMouse);

    PostSetLED(2, false);
    PostSetLED(5, false);
    PostSetLED(8, false);
  }
}

void Adafruit_IntelliKeys::SweepSound(int iStartFreq, int iEndFreq,
                                      int iDuration) {
  uint8_t report[8] = {IK_CMD_TONE, 0, 0, 0, 0, 0, 0, 0};

  int volume = 2;

  int nLight = 0;
  bool bOn = true;
  int j = 0;

  int iLoops = iDuration / 5; // 5-msec chunks
  for (int i = 0; i < iLoops; i++) {
    report[1] = iStartFreq + i * ((iEndFreq - iStartFreq) * 100 / iLoops) / 100;
    report[2] = volume;
    PostCommand(report);

    j++;
    if (j == 5) {
      j = 0;
      nLight++;
      if (nLight > 9) {
        bOn = !bOn;
        nLight = 1;
      }
      PostSetLED(nLight, bOn);
    }
  }

  report[2] = 0;
  PostCommand(report);

  //  restore lights
  for (int i2 = 0; i2 < 9; i2++) {
    PostSetLED(i2 + 1, false);
  }
}

void Adafruit_IntelliKeys::PostReportDataToControlPanel(bool bForce) {
#if 0
  PostDelay(5);
  uint8_t command[IK_REPORT_LEN] = {
      IK_CMD_CP_REPORT_REALTIME, bForce, 0, 0, 0, 0, 0, 0};
  PostCommand(command);
#endif
}

void Adafruit_IntelliKeys::OnToggle(int newValue) {
  if (m_toggle != newValue) {
    // Reset state (command, input queues)
    // PurgeQueues();

    m_toggle = newValue;

    int freqLow = 200;
    int freqHigh = 250;
    int duration = 200;

    if (m_toggle == 1) {
      SweepSound(freqLow, freqHigh, duration);
    } else {
      SweepSound(freqHigh, freqLow, duration);
    }

    //  reset keyboard
    ResetKeyboard();

    //  reset mouse
    ResetMouse();

    // IKControlPanel::Refresh();

    if (_toggle_cb) {
      _toggle_cb((uint8_t)m_toggle);
    }
  }
}

void Adafruit_IntelliKeys::OnSwitch(int nswitch, int state) {
  m_switches[nswitch - 1] = state;
}

void Adafruit_IntelliKeys::OnSensorChange(int sensor, int value) {
  uint32_t now = millis();

  //  save the current sensor value
  m_sensors[sensor] = value;

  //  what's the new overlay value
  int v[IK_NUM_SENSORS];

  for (int i = 0; i < IK_NUM_SENSORS; i++) {
    v[i] = 0;
    int midway;
    if (m_bEepromValid) {
      midway = (50 * m_eepromData.sensorBlack[i] +
                50 * m_eepromData.sensorWhite[i]) /
               100;
    } else {
      midway = 150;
    }

    if (m_sensors[i] > midway) {
      v[i] = 1;
    }
  }

  int newVal = 0;
  for (int i = 0; i < IK_NUM_SENSORS; i++) {
    newVal += v[i] * (1 << i);
  }

  //  if the value changed, record what and when
  if (newVal != m_lastOverlay) {
    m_lastOverlay = newVal;
    m_lastOverlayTime = now;
  }
}

void Adafruit_IntelliKeys::ProcessInput(uint8_t const *data, uint8_t len) {
  uint8_t const event_id = data[0];
#if IK_DEBUG
  // skip print sensor change since it is a lot
  if (event_id != IK_EVENT_SENSOR_CHANGE) {
    IK_PRINTF2("Event: %s: ", ik_event_str[event_id - EVENT_BASE]);
    for (uint8_t i = 0; i < len; i++) {
      IK_PRINTF2("%02x ", data[i]);
    }
    IK_PRINTF2("\n");
  }
#endif

  switch (event_id) {
  case IK_EVENT_MEMBRANE_PRESS:
    OnMembranePress(data[1], data[2]);
    InterpretRaw();
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_MEMBRANE_RELEASE:
    OnMembraneRelease(data[1], data[2]);
    InterpretRaw();
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_SWITCH:
    OnSwitch(data[1], data[2]);
    InterpretRaw();
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_SENSOR_CHANGE:
    OnSensorChange(data[1], data[2]);
    break;

  case IK_EVENT_VERSION:
    // JR - June 2012 - set the firmware version
    m_firmwareVersionMajor = data[1];
    m_firmwareVersionMinor = data[2];
    break;

  case IK_EVENT_EEPROM_READ:
    break;

  case IK_EVENT_ONOFFSWITCH:
    OnToggle(data[1]);
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_CORRECT_MEMBRANE:
    OnCorrectMembrane(data[1], data[2]);
    InterpretRaw();
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_CORRECT_SWITCH:
    OnCorrectSwitch(data[1]);
    InterpretRaw();
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_CORRECT_DONE:
    OnCorrectDone();
    PostReportDataToControlPanel(true);
    break;

  case IK_EVENT_EEPROM_READBYTE:
    StoreEEProm(data[1], data[2], data[3]);
    break;

  case IK_EVENT_AUTOPILOT_STATE:
    break;

  default:
    break;

    //  We should not see these.

  case IK_EVENT_ACK:
  case IK_EVENT_DEVICEREADY:
  case IK_EVENT_NOMOREEVENTS:
  case IK_EVENT_MEMBRANE_REPEAT:
  case IK_EVENT_SWITCH_REPEAT:
    //  error??
    break;
  }
}

void Adafruit_IntelliKeys::PostLiftAllModifiers() {
  //  send the command
  uint8_t report[IK_REPORT_LEN] = {
      IK_CMD_LIFTALLMODIFIERS, 0, 0, 0, 0, 0, 0, 0};
  PostCommand(report);
}

void Adafruit_IntelliKeys::PostCPRefresh() {
  uint8_t command[IK_REPORT_LEN] = {IK_CMD_CP_REFRESH, 0, 0, 0, 0, 0, 0, 0};
  PostCommand(command);
}

void Adafruit_IntelliKeys::ResetKeyboard(void) {
  //  reset keyboard
  //  reconcile with modifier objects?
  PostLiftAllModifiers();
}

void Adafruit_IntelliKeys::ResetMouse(void) {
  //  reset mouse
}

void Adafruit_IntelliKeys::OnStdOverlayChange() {
  ResetKeyboard();
  ResetMouse();
  PostLiftAllModifiers();
  // SetLevel(1);

#if 0
  if (DATAI(TEXT("Reload_Standard_Overlay_When_Recognized"),0)==1)
  {
    IKEngine::GetEngine()->LoadStandardOverlays();
  }
#endif

  LongKeySound();
  OverlayRecognitionFeedback();

  //  tell the CP
  PostCPRefresh();

  // tell raw mode
#if 0
  bool bRaw = IKEngine::GetEngine()->GetRawMode();
  if (bRaw) {
    uint32_t time = millis();
    queueEntry qe;
    qe.buffer[0] = 3;   //  event type

    if (m_currentOverlay == 7) {
      qe.buffer[1] = 0;    //  overlay number
    }
    else {
      qe.buffer[1] = m_currentOverlay + 1;    //  overlay number
    }

    qe.buffer[2] = 0;    //  unused
    qe.buffer[3] = 0;    //  unused
    *((unsigned int *)&(qe.buffer[4])) = time;
    m_rawQueue.enqueue(qe);
  }
#endif
}

bool Adafruit_IntelliKeys::HasStandardOverlay() {
  // 0-6 is standard overlay, 7 is no overlay
  return (0 <= m_currentOverlay && m_currentOverlay < 7);
}

IKOverlay *Adafruit_IntelliKeys::GetCurrentOverlay() {
  if (HasStandardOverlay()) {
    return &stdOverlays[m_currentOverlay];
  } else if ((m_currentOverlay > 7) &&
             (m_currentOverlay - 8 < _custom_overlay_count) &&
             (_custom_overlay != NULL)) {
    return &_custom_overlay[m_currentOverlay - 8];
  } else {
    return NULL;
  }
}

void Adafruit_IntelliKeys::OverlayRecognitionFeedback() {
  // PostMonitorState(false);

  int delay = 300;

  if (IsSwitchedOn()) {
    PostSetLED(1, true);
    PostSetLED(4, true);
    PostSetLED(7, true);
    PostDelay(delay);
    PostSetLED(1, false);
    PostSetLED(4, false);
    PostSetLED(7, false);

    PostSetLED(2, true);
    PostSetLED(5, true);
    PostSetLED(8, true);
    PostDelay(delay);
    PostSetLED(2, false);
    PostSetLED(5, false);
    PostSetLED(8, false);

    PostSetLED(3, true);
    PostSetLED(6, true);
    PostSetLED(9, true);
    PostDelay(delay);
    PostSetLED(3, false);
    PostSetLED(6, false);
    PostSetLED(9, false);
  } else {
    for (int numFlashes = 0; numFlashes < 6; numFlashes++) {
      PostSetLED(2, true);
      PostSetLED(5, true);
      PostSetLED(8, true);
      PostDelay(delay);
      // PostLedReconcile();

      PostDelay(delay);

      PostSetLED(2, false);
      PostSetLED(5, false);
      PostSetLED(8, false);
      PostDelay(delay);
      // PostLedReconcile();

      PostDelay(delay);
    }
  }

  // PostMonitorState(true);

  // PostLedReconcile();
}

int Adafruit_IntelliKeys::GetLevel() { return m_currentLevel; }

void Adafruit_IntelliKeys::SetLevel(int level) { m_currentLevel = level; }

void Adafruit_IntelliKeys::SettleOverlay() {
  uint32_t now = millis();

  //  settle overlay
  if (m_lastOverlay != m_currentOverlay && now > m_lastOverlayTime + 1000) {
    m_currentOverlay = m_lastOverlay;
    IK_PRINTF("Settled on overlay %d\n", m_currentOverlay);

    SetLevel(1);

    OnStdOverlayChange();
  }
}

void Adafruit_IntelliKeys::ShortKeySound() { KeySound(50); }

void Adafruit_IntelliKeys::LongKeySound() { KeySound(700); }

void Adafruit_IntelliKeys::KeySound(int msLength) {
  KeySoundVol(msLength, IKSettings::GetSettings()->m_iKeySoundVolume);
}

void Adafruit_IntelliKeys::KeySoundVol(int msLength, int vol) {
  int myVol = vol;
  if (vol == -1) {
    myVol = IKSettings::GetSettings()->m_iKeySoundVolume;
  }

  //  set parameters and blow
  uint8_t report[IK_REPORT_LEN] = {IK_CMD_TONE, 0, 0, 0, 0, 0, 0, 0};
  report[1] = 247;
  report[2] = myVol;
  report[3] = msLength / 10;
  PostCommand(report);
}

void Adafruit_IntelliKeys::StoreEEProm(uint8_t data, uint8_t add_lsb,
                                       uint8_t add_msb) {
  //  store the uint8_t received;
  int ndx = add_lsb - 0x80;

  uint8_t *e = (uint8_t *)&m_eepromData;
  e[ndx] = data;

  //  mark the uint8_t valid;
  m_eepromDataValid[ndx] = true;

  //  check to see if all the uint8_ts are valid.
  //  if so, say we're valid and refresh the
  //  control panel.
  int nInvalid = 0;
  for (unsigned int i = 0; i < sizeof(eeprom_t); i++) {
    if (!m_eepromDataValid[i]) {
      nInvalid++;
    }
  }

  if (nInvalid == 0 && !m_bEepromValid) {
    if (m_eepromData.serialnumber[0] == 'C' &&
        m_eepromData.serialnumber[1] == '-') {
      m_bEepromValid = true;
      IK_PRINTF("EEPROM data valid\n");
      PostCPRefresh();
    }
  }
}

void Adafruit_IntelliKeys::hid_reprot_received_cb(uint8_t daddr, uint8_t idx,
                                                  uint8_t const *report,
                                                  uint16_t len) {
  // IDX 0 is HID Generic
  if (idx != 0) {
    return;
  }

  if (len != IK_REPORT_LEN) {
    IK_PRINTF("Invalid report length %d\n", len);
    return;
  }

  ProcessInput(report, len);

  if (!tuh_hid_receive_report(daddr, idx)) {
    IK_PRINTF("Failed to receive report\n");
    return;
  }
}

//--------------------------------------------------------------------+
// EZUSB
//--------------------------------------------------------------------+

// Make an firmware load control request
bool Adafruit_IntelliKeys::ezusb_load_xfer(uint8_t bRequest, uint16_t addr,
                                           const void *buffer, uint16_t len) {
  tusb_control_request_t const request = {
      .bmRequestType_bit = {.recipient = TUSB_REQ_RCPT_DEVICE,
                            .type = TUSB_REQ_TYPE_VENDOR,
                            .direction = TUSB_DIR_OUT},
      .bRequest = bRequest,
      .wValue = tu_htole16(addr),
      .wIndex = 0,
      .wLength = tu_htole16(len),
  };

  tuh_xfer_t xfer = {.daddr = _daddr,
                     .ep_addr = 0,
                     .setup = &request,
                     .buffer = (uint8_t *)buffer,
                     .complete_cb = NULL,
                     .user_data = 0};

  if (!tuh_control_xfer(&xfer)) {
    return false;
  }

  return xfer.result == XFER_RESULT_SUCCESS;
}

bool Adafruit_IntelliKeys::ezusb_StartDevice(void) {
  IK_PRINTF("Downloading firmware\n");

  xfer_result_t result = XFER_RESULT_INVALID;
  tuh_interface_set(_daddr, 0, 0, NULL, (uintptr_t)&result);

  if (result != XFER_RESULT_SUCCESS) {
    IK_PRINTF("Failed to Set Interface\n");
    return false;
  }

  // First download loader firmware. The loader firmware implements a vendor
  // specific command that will allow us to anchor load to external ram
  ezusb_8051Reset(1);
  ezusb_DownloadIntelHex(ik_loader);
  ezusb_8051Reset(0);

  // Now download the device firmware
  ezusb_DownloadIntelHex(ik_firmware);
  ezusb_8051Reset(1);
  ezusb_8051Reset(0);

  IK_PRINTF("Downloaded firmware\n");

  return true;
}

bool Adafruit_IntelliKeys::ezusb_DownloadIntelHex(
    INTEL_HEX_RECORD const *hex_record) {
  // The download must be performed in two passes.  The first pass loads all of
  // the external addresses, and the 2nd pass loads to all of the internal
  // addresses. why?  because downloading to the internal addresses will
  // probably wipe out the firmware running on the device that knows how to
  // receive external ram downloads.

  // First download all the records that go in external ram
  ezusb_downloadHex(hex_record, false);

  // Now download all of the records that are in internal RAM.  Before starting
  // the download, stop the 8051.
  ezusb_8051Reset(1);

  ezusb_downloadHex(hex_record, true);

  return false;
}

bool Adafruit_IntelliKeys::ezusb_downloadHex(INTEL_HEX_RECORD const *ptr,
                                             bool internal_ram) {
  // First download all the records that go in external ram
  while (ptr->Type == 0) {
    if (INTERNAL_RAM(ptr->Address) == internal_ram) {
      // IK_PRINTF("Downloading %d uint8_ts to 0x%x\n", ptr->Length,
      // ptr->Address);

      uint8_t const bRequest =
          internal_ram ? ANCHOR_LOAD_INTERNAL : ANCHOR_LOAD_EXTERNAL;
      if (!ezusb_load_xfer(bRequest, ptr->Address, ptr->Data, ptr->Length)) {
        IK_PRINTF("Failed to load hex file");
        return false;
      }
    }

    ptr++;
  }

  return true;
}

bool Adafruit_IntelliKeys::ezusb_8051Reset(uint8_t resetBit) {
  return ezusb_load_xfer(ANCHOR_LOAD_INTERNAL, CPUCS_REG, &resetBit, 1);
}
