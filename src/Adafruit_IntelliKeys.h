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

#include "IKModifier.h"
#include "IKOverlay.h"
#include "IKUniversal.h"

//  maximum numbers
#define MAX_INTELLIKEYS 10
#define MAX_STANDARD_OVERLAYS 8
#define MAX_SWITCH_OVERLAYS 30

#define IK_CMD_FIFO_SIZE 128

class Adafruit_IntelliKeys {
public:
  typedef void (*membrane_callback_t)(uint8_t row, uint8_t col, uint8_t state);
  typedef void (*switch_callback_t)(uint8_t sw, uint8_t state);
  typedef void (*toggle_callback_t)(uint8_t state);

  Adafruit_IntelliKeys(void);

  //--------------------------------------------------------------------+
  // Arduino API
  //--------------------------------------------------------------------+

  void begin(void);
  bool mount(uint8_t daddr);
  void umount(uint8_t daddr);

  void setCustomOverlay(IKOverlay *overlay, uint32_t count) {
    _custom_overlay = overlay;
    _custom_overlay_count = count;
  }

  void getHIDReport(hid_keyboard_report_t *kb_report,
                    hid_mouse_report_t *mouse_report);
  void Periodic(void);

  void onMemBraneChanged(membrane_callback_t func) { _membrane_cb = func; }
  void onSwitchChanged(switch_callback_t func) { _switch_cb = func; }
  void onToggleChanged(toggle_callback_t func) { _toggle_cb = func; }

  uint8_t const (*getMembrane(void))[IK_RESOLUTION_Y] { return m_membrane; }

  //--------------------------------------------------------------------+
  // Function named following IKDevice in OpenIKeys
  //--------------------------------------------------------------------+

  bool isAttached(void) { return _daddr != 0; }
  bool IsOpen(void) { return _opened; }
  bool IsSwitchedOn(void) { return m_toggle == 1; }
  bool IsNumLockOn(void);
  bool IsCapsLockOn(void);
  bool IsMouseDown(void);

  bool PostCommand(uint8_t *command);
  void PostDelay(uint8_t msec);
  void PostSetLED(uint8_t number, uint8_t value);
  void PostKey(int code, int direction, int delayAfter = 0);
  void PostLiftAllModifiers(void);
  void PostCPRefresh();
  void PostReportDataToControlPanel(bool bForce = false);
  void ProcessCommands();

  void OnToggle(int newValue);
  void OnSwitch(int nswitch, int state);
  void OnSensorChange(int sensor, int value);
  void StoreEEProm(uint8_t data, uint8_t add_lsb, uint8_t add_msb);
  void ProcessInput(uint8_t const *data, uint8_t len);

  void SetLEDs(void);
  void SweepSound(int iStartFreq, int iEndFreq, int iDuration);

  void OnMembranePress(int x, int y);
  void OnMembraneRelease(int x, int y);
  void OnCorrectSwitch(int switchnum);
  void DoCorrect();
  void OnCorrectMembrane(int x, int y);
  void OnCorrectDone();

  void ResetKeyboard(void);
  void ResetMouse(void);

  int GetLevel();
  void SetLevel(int level);
  int GetCurrentOverlayNumber() { return m_currentOverlay; }
  bool HasStandardOverlay();
  IKOverlay *GetCurrentOverlay();
  void SettleOverlay();
  void OnStdOverlayChange();
  void OverlayRecognitionFeedback();
  int GetDevType() { return 1; /* 1 is IntelliKeys */ }

  void ShortKeySound();
  void LongKeySound();
  void KeySound(int msLength);
  void KeySoundVol(int msLength, int volume = -1);
  void InterpretRaw();

  void hid_reprot_received_cb(uint8_t dev_addr, uint8_t instance,
                              uint8_t const *report, uint16_t len);

private:
  uint8_t _daddr;
  uint8_t _opened;

  membrane_callback_t _membrane_cb;
  switch_callback_t _switch_cb;
  toggle_callback_t _toggle_cb;

  IKOverlay *_custom_overlay;
  uint32_t _custom_overlay_count;

  //------------- From OpenIKeys -------------//

  int m_currentLevel;
  int m_newLevel;

  uint32_t m_lastLEDTime;
  uint32_t m_delayUntil;
  uint32_t m_nextCorrect;

  int m_toggle; // on/off switch
  int m_sensors[IK_NUM_SENSORS];

  int m_lastSwitch;

  //  overlay recognition
  int m_lastOverlay;
  uint32_t m_lastOverlayTime;
  int m_currentOverlay;

  //  reading the eeprom
  eeprom_t m_eepromData;
  unsigned int m_eepromRequestTime[sizeof(eeprom_t)];
  bool m_eepromDataValid[sizeof(eeprom_t)];
  bool m_bEepromValid;

  //  for correction
  uint8_t m_membranePressedInCorrectMode[IK_RESOLUTION_X][IK_RESOLUTION_Y];
  uint8_t m_switchesPressedInCorrectMode[IK_NUM_SWITCHES];

  uint8_t m_last_membrane[IK_RESOLUTION_X][IK_RESOLUTION_Y];
  uint8_t m_last_switches[IK_NUM_SWITCHES];

  uint8_t m_membrane[IK_RESOLUTION_X][IK_RESOLUTION_Y];
  uint8_t m_switches[IK_NUM_SWITCHES];

  uint8_t m_firmwareVersionMajor;
  uint8_t m_firmwareVersionMinor;

  int m_lastCodeUp;
  bool m_bShifted;

  IKModifier m_modShift;
  IKModifier m_modAlt;
  IKModifier m_modControl;
  IKModifier m_modCommand;

  IKModifier m_mouseDown;

  tu_fifo_t _cmd_ff;
  OSAL_MUTEX_DEF(_cmd_ff_mutex);
  uint8_t _cmd_ff_buf[8 * IK_CMD_FIFO_SIZE];

  bool Start(void);
  void Reset(void);

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
