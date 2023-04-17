/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Ha Thach (thach@tinyusb.org) for Adafruit Industries
 */

#include "Arduino.h"

#include "Adafruit_IntelliKeys.h"
#include "IKModifier.h"

#define IK_MODIFIER_CHANGE_DELAY 5

void IKModifier::ToggleState() {
  // skip if it is changed in the last 5ms
  if (millis() - m_lastTime > IK_MODIFIER_CHANGE_DELAY) {
    m_lastTime = millis();
    m_state = (m_state == kModifierStateOff) ? kModifierStateLatched
                                             : kModifierStateOff;
  }
}

void IKModifier::UpdateState(uint8_t mask) {
  // skip if it is changed in the last 5ms
  if (millis() - m_lastTime > IK_MODIFIER_CHANGE_DELAY) {
    m_lastTime = millis();

    if (mask & m_mask) {
      // toggle if it is our mask
      m_state = (m_state == kModifierStateOff) ? kModifierStateLatched
                                               : kModifierStateOff;
    }
  }
}

void IKModifier::SetState(int state) { m_state = state; }

void IKModifier::Execute(int code) {
  int theCode = m_universalCode;
  m_universalCodeOverride = 0;
  if (code != 0) {
    theCode = code;
    m_universalCodeOverride = theCode;
  }

  IKSettings *pSettings = IKSettings::GetSettings();
  if (pSettings == NULL)
    return;

  switch (pSettings->m_iShiftKeyAction) {
  case kSettingsShiftLatching:
    if (m_state == kModifierStateOff) {
      //  was off, turn on and put down
      m_device->PostKey(theCode, IK_DOWN);
      m_state = kModifierStateLatched;
    } else {
      //  was on, put up and turn off
      m_device->PostKey(theCode, IK_UP);
      m_state = kModifierStateOff;
    }
    break;

  case kSettingsShiftLocking:
    if (m_state == kModifierStateOff) {
      //  put key down, state = latched
      m_device->PostKey(theCode, IK_DOWN);
      m_state = kModifierStateLatched;
    } else if (m_state == kModifierStateLatched) {
      //  leave key alone, state = locked
      m_state = kModifierStateLocked;
    } else if (m_state == kModifierStateLocked) {
      //  put key up, state = off
      m_device->PostKey(theCode, IK_UP);
      m_state = kModifierStateOff;
    }
    break;

  case kSettingsShiftNoLatch:
    //  just put the key down/up, no change in state
    {
      int keyDownDelay = 5;
      m_device->PostKey(theCode, IK_DOWN, keyDownDelay);
      // m_device->PostKey(theCode,IK_UP,keyDownDelay);
      m_state = kModifierStateLatched;
    }
    break;

  default:
    break;
  }
}
