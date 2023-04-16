/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Ha Thach (thach@tinyusb.org) for Adafruit Industries
 */

#ifndef ADAFRUIT_INTELLIKEYS_IKMODIFIER_H
#define ADAFRUIT_INTELLIKEYS_IKMODIFIER_H

#include "IKSettings.h"
#include <stdint.h>
#include <string.h>

//  for keystrokes and mouse buttons
enum { IK_DOWN = 1, IK_UP, IK_TOGGLE };
enum {
  IKUSB_LEFT_BUTTON = 0,
  IKUSB_RIGHT_BUTTON,
  IKUSB_MIDDLE_BUTTON,
  IKUSB_BUTTON_4,
  IKUSB_BUTTON_5,
  IKUSB_BUTTON_6,
  IKUSB_BUTTON_7,
  IKUSB_BUTTON_8
};

enum { kModifierStateOff = 0, kModifierStateLatched, kModifierStateLocked };

class Adafruit_IntelliKeys;

class IKModifier {

public:
  IKModifier() {
    m_state = kModifierStateOff;
    m_lastTime = 0;
  }
  virtual ~IKModifier() {}
  void Execute(int code = 0);
  int GetState() { return m_state; }
  void SetState(int state);
  void SetCode(uint8_t code) { m_universalCode = code; }

  void ToggleState();

  int m_state;
  int m_universalCode;
  int m_universalCodeOverride;
  uint32_t m_lastTime;

  void SetDevice(Adafruit_IntelliKeys *pDev) { m_device = pDev; }
  Adafruit_IntelliKeys *m_device;
};

#endif // ADAFRUIT_INTELLIKEYS_IKMODIFIER_H
