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

#ifndef ADAFRUIT_INTELLIKEYS_H_
#define ADAFRUIT_INTELLIKEYS_H_

#include "Adafruit_TinyUSB.h"
#include "intellikeysdefs.h"

class Adafruit_IntelliKeys {
public:
  Adafruit_IntelliKeys(void);

  void begin(void);
  bool mount(uint8_t daddr);
  void umount(uint8_t daddr);

  void hid_reprot_received_cb(uint8_t dev_addr, uint8_t instance,
                              uint8_t const *report, uint16_t len);

private:
  uint8_t _daddr;
  uint8_t _state;

  uint8_t m_KeyBoardReport[7];
  uint8_t m_MouseReport[3];
  int m_toggle; // on/off switch
  int m_switches[IK_NUM_SWITCHES];
  int m_sensors[IK_NUM_SENSORS];

  //  overlay recognition
  int m_lastOverlay;
  uint32_t m_lastOverlayTime;
  int m_currentOverlay;

  //  reading the eeprom
  eeprom_t m_eepromData;
  bool m_bEepromValid;

  uint8_t m_firmwareVersionMajor;
  uint8_t m_firmwareVersionMinor;

  // Function named following IKDevice in OpenIKeys
  bool Start(void);

  bool PostCommand(uint8_t *command);
  void ProcessInput(uint8_t const *data, uint8_t len);

  void OnToggle(int newValue);
  void OnSwitch(int nswitch, int state);
  void OnSensorChange(int sensor, int value);

  void SweepSound(int iStartFreq, int iEndFreq, int iDuration);
  void PostSetLED(int number, int value);

  // ezusb
  bool ezusb_StartDevice(void);
  bool ezusb_DownloadIntelHex(INTEL_HEX_RECORD const *record);
  bool ezusb_8051Reset(uint8_t resetBit);

  // internal helper
  bool ezusb_load_xfer(uint8_t brequest, uint16_t addr, const void *buffer,
                       uint16_t len);
  bool ezusb_downloadHex(INTEL_HEX_RECORD const *record, bool internal_ram);
};

#endif
