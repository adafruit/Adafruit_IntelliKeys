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

//--------------------------------------------------------------------+
// Public API
//--------------------------------------------------------------------+

Adafruit_IntelliKeys::Adafruit_IntelliKeys(void) {
  _daddr = 0;
  _state = 0;

  m_lastOverlay = -1;
  m_lastOverlayTime = 0;
  m_currentOverlay = -1;

  m_toggle = -1;

  m_bEepromValid = false;

  m_firmwareVersionMajor = 0;
  m_firmwareVersionMinor = 0;
}

void Adafruit_IntelliKeys::begin(void) {}

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
    _daddr = 0;
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

  // PostDelay ( 250);

  command[0] = IK_CMD_ALL_SENSORS;
  command[1] = 0; //  unused
  PostCommand(command);

  // ask the device for its firmware version
  command[0] = IK_CMD_GET_VERSION;
  command[1] = 0; //  unused
  PostCommand(command);

  return false;
}

bool Adafruit_IntelliKeys::PostCommand(uint8_t *command) {
  uint8_t idx = 0;

  IK_PRINTF("PostCommand: %s\r\n", ik_cmd_str[command[0]]);

  // blocking for now
  while (!tuh_hid_send_report(_daddr, idx, 0, command, IK_REPORT_LEN)) {
    tuh_task();
  }
  return false;
}

void Adafruit_IntelliKeys::PostSetLED(int number, int value) {
  uint8_t command[IK_REPORT_LEN] = {IK_CMD_LED, number, value, 0, 0, 0, 0, 0};
  PostCommand(command);
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
    // ResetKeyboard();

    //  reset mouse
    // ResetMouse();

    //  reset level
    // SetLevel(1);

    // IKControlPanel::Refresh();
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
  IK_PRINTF("Event: %s: ", ik_event_str[event_id - EVENT_BASE]);
#if IK_DEBUG
  for (uint8_t i = 0; i < len; i++) {
    IK_PRINTF("%02x ", data[i]);
  }
  IK_PRINTF("\n");
#endif

  switch (event_id) {
#if 0
    case IK_EVENT_MEMBRANE_PRESS:
      OnMembranePress(qe.buffer[1],qe.buffer[2]);
      if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
        InterpretRaw();
      PostReportDataToControlPanel();
      break;

    case IK_EVENT_MEMBRANE_RELEASE:
      OnMembraneRelease(qe.buffer[1],qe.buffer[2]);
      if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
        InterpretRaw();
      PostReportDataToControlPanel();
      break;

    case IK_EVENT_SWITCH:
      OnSwitch(qe.buffer[1],qe.buffer[2]);
      if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
        InterpretRaw();
      PostReportDataToControlPanel();
      break;
#endif

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
    // PostReportDataToControlPanel();
    break;

#if 0
    case IK_EVENT_CORRECT_MEMBRANE:
      OnCorrectMembrane(qe.buffer[1],qe.buffer[2]);
      if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
        InterpretRaw();
      PostReportDataToControlPanel();
      break;

    case IK_EVENT_CORRECT_SWITCH:
      OnCorrectSwitch(qe.buffer[1]);
      if (IKEngine::GetEngine()->UseRawMode()&&!HasStandardOverlay()&&!IKEngine::GetEngine()->GetDiagnosticMode())
        InterpretRaw();
      PostReportDataToControlPanel();
      break;

    case IK_EVENT_CORRECT_DONE:
      OnCorrectDone();
      PostReportDataToControlPanel(true);
      break;

    case IK_EVENT_EEPROM_READBYTE:
      StoreEEProm(qe.buffer[1],qe.buffer[2],qe.buffer[3]);
      break;

    case IK_EVENT_AUTOPILOT_STATE:
      break;
#endif

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
      // IK_PRINTF("Downloading %d bytes to 0x%x\n", ptr->Length, ptr->Address);

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
