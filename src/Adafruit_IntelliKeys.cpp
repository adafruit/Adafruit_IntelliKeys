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

//--------------------------------------------------------------------+
// Public API
//--------------------------------------------------------------------+

Adafruit_IntelliKeys::Adafruit_IntelliKeys(void) {
  _daddr = 0;
  _opened = false;
  _expected_resp = 0;

  m_lastLEDTime = 0;
  m_delayUntil = 0;
  m_nextCorrect = 0;

  m_newLevel = 0;
  m_currentLevel = 0;

  m_lastOverlay = -1;
  m_lastOverlayTime = 0;
  m_currentOverlay = -1;

  m_toggle = -1;

  for (int x = 0; x < IK_RESOLUTION_X; x++) {
    for (int y = 0; y < IK_RESOLUTION_Y; y++) {
      m_membrane[y][x] = 0;
    }
  }

  m_bEepromValid = false;

  m_firmwareVersionMajor = 0;
  m_firmwareVersionMinor = 0;

  m_lastExecuted = NULL;

  m_lastSwitch = 0;

  for (int i2 = 0; i2 < 5; i2++) {
    m_last5Overlays[i2] = -1;
  }

  for (unsigned int i3 = 0; i3 < sizeof(m_KeyBoardReport); i3++) {
    m_KeyBoardReport[i3] = 0;
  }

  for (unsigned int i4 = 0; i4 < sizeof(m_MouseReport); i4++) {
    m_MouseReport[i4] = 0;
  }

  tu_fifo_config(&_cmd_ff, _cmd_ff_buf, IK_CMD_FIFO_SIZE, 8, false);
  tu_fifo_config_mutex(&_cmd_ff, osal_mutex_create(&_cmd_ff_mutex), NULL);
}

void Adafruit_IntelliKeys::Reset(void) {}

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

void Adafruit_IntelliKeys::Periodic(void) {
  if (!IsOpen()) {
    return; // nothing to do
  }

  // settle overlay
  SettleOverlay();

  uint32_t now = millis();

  //  setLEDs
  if (now > m_lastLEDTime + 100) {
    // SetLEDs();
    m_lastLEDTime = now;
  }

  //  request a correction every so often.
  if (now > m_nextCorrect) {
    // DoCorrect();
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

  if (HasStandardOverlay()) {
    Interpret();
  } else {
    InterpretRaw();
  }
}

void Adafruit_IntelliKeys::InterpretRaw() {}

void Adafruit_IntelliKeys::FindDomain(IKOverlay *pOverlay, int *domainNumber,
                                      bool *bPressAnywhere, int *switchNumber) {
  *bPressAnywhere = false;
  *switchNumber = 0;
  *domainNumber = 0;

  //  first check membrane by column and then row,
  //  both in reverse order.  This will favor
  //  cells that are farther to the left and higher.

  int col, row;
  {
    for (col = 0; col < IK_RESOLUTION_X; col++) {
      for (row = 0; row < IK_RESOLUTION_Y; row++) {
        if (m_membrane[IK_RESOLUTION_Y - 1 - row][IK_RESOLUTION_X - 1 - col]) {
          int nd;
          nd = pOverlay->GetDomainFromMembrane(
              IK_RESOLUTION_X - 1 - col, IK_RESOLUTION_Y - 1 - row, GetLevel());
          if (nd > 0) {
            *domainNumber = nd;
            *bPressAnywhere = true;
            return;
          }
          *bPressAnywhere = true;
        }
      }
    }
  }

  //  which switches are down?

  int down[IK_NUM_SWITCHES] = {0, 0, 0, 0, 0, 0};
  int domain[IK_NUM_SWITCHES] = {0, 0, 0, 0, 0, 0};

  int j;
  int nDown = 0;
  for (j = 0; j < IK_NUM_SWITCHES; j++)
    if (m_switches[j]) {
      down[nDown] = j + 1;
      domain[nDown] = pOverlay->GetDomainFromSwitch(j + 1, GetLevel());
      nDown++;
    }

  //  if the same one is still down since last time, just return it.

  if (m_lastSwitch > 0) {
    for (j = 0; j < nDown; j++) {
      if (m_lastSwitch == down[j]) {
        *domainNumber = domain[j];
        *bPressAnywhere = true;
        *switchNumber = m_lastSwitch;
        return;
      }
    }
  }

  //  find the new down.

  m_lastSwitch = 0;
  static bool bForward = true;

  for (j = 0; j < nDown; j++) {
    bForward = !bForward;
    int jj = j;
    if (!bForward)
      jj = nDown - j - 1;

    if (down[jj] > 0) {
      m_lastSwitch = down[jj];
      *domainNumber = domain[jj];
      *bPressAnywhere = true;
      *switchNumber = m_lastSwitch;
      return;
    }
  }
}

IKOverlay *Adafruit_IntelliKeys::GetStandardOverlay(int index) {
  if (index >= 0 && index <= MAX_STANDARD_OVERLAYS) {
    // return &(m_standardOverlay[index]);
    return NULL;
  }
  return NULL;
}

IKOverlay *Adafruit_IntelliKeys::GetCurrentOverlay() {
  IKOverlay *pOv = NULL;

  //  always use standard overlay if there is one
  if (HasStandardOverlay()) {
    return GetStandardOverlay(m_currentOverlay);
  }

  // TODO user overlay

  return NULL;
}

void Adafruit_IntelliKeys::Interpret() {
  enum { kPress = 0, kRelease, kRepeat, kSlide };

  //  don't bother if we're not connected and switched on
  if (!IsOpen()) {
    return;
  }

  if (!IsSwitchedOn()) {
    return;
  }

  //  don't bother if there is no current overlay
  IKOverlay *pOverlay = GetCurrentOverlay();
  if (pOverlay == NULL)
    return;

  //  find current settings
  IKSettings *pSettings = IKSettings::GetSettings();

  //  analyze switches and membrane.
  int newDomain = 0;
  bool PressAnywhere = false;
  // bool bWasSwitch = false;
  int switchNumber;

  FindDomain(pOverlay, &newDomain, &PressAnywhere, &switchNumber);

#if 0
  if (m_newLevel>0 && !PressAnywhere)
  {
    SetLevel(m_newLevel);
    m_newLevel = 0;
  }

  //  get pointers to the new and repeating (if any) data
  BYTE *newDomainData     = pOverlay->GetUniversalCodesFromDomain(newDomain,GetLevel());
  BYTE *repeatDomainData  = pOverlay->GetUniversalCodesFromDomain(m_repeatDomain,GetLevel());
  BYTE *currentDomainData = pOverlay->GetUniversalCodesFromDomain(m_currentDomain,GetLevel());

  //  sanity
  if (newDomain!=0 && newDomainData==NULL)
    newDomain = 0;
  if (m_repeatDomain!=0 && repeatDomainData==NULL)
    m_repeatDomain = 0;
  if (m_currentDomain!=0 && currentDomainData==NULL)
    m_currentDomain = 0;

  unsigned int now = IKUtil::GetCurrentTimeMS();


  //  play dead until the time arrives
  IKASSERT(now+1000 >= m_deadUntil);
  if ( now < m_deadUntil)
    return;

  //  what is the membrane doing?

  int stage = -1;
  if (m_currentDomain == 0 && newDomain != 0)
    stage = kPress;
  if (m_currentDomain != 0 && newDomain == 0)
    stage = kRelease;
  if (m_currentDomain != 0 && newDomain != 0 && m_currentDomain==newDomain)
    stage = kRepeat;
  if (m_currentDomain != 0 && newDomain != 0 && m_currentDomain!=newDomain)
    stage = kSlide;

  //  handle special setup keys here and bail
  if (pOverlay->IsSetupOverlay())
  {
    if (!m_bDidSetup)
    {
      if ((stage == kPress || stage==kSlide))
      {
        m_bDidSetup = DoSetupKey(newDomainData);
        if (m_bDidSetup)
        {
          m_currentDomain = newDomain;
          return;
        }
      }
    }
    else
    {
      //  setup key in progress, wait for release
      if (stage==kRelease || stage==-1)
        m_bDidSetup = false;
      m_currentDomain = newDomain;
      return;
    }
  }

  if (stage == kPress || stage == kSlide)
    m_lastPress = now;
  if (stage == kRelease)
    m_lastRelease = now;

  //  handle repeat latching
  if (m_bRepeatLatched && stage == kRelease)
    m_bRepeatLatchReleased = true;
  if (m_bRepeatLatched && m_bRepeatLatchReleased && PressAnywhere)
  {
    PurgeQueues();
    ResetKeyboard();
    ShortKeySound();
    m_bRepeatLatched = false;
    newDomain = 0;
    stage = -1;
    m_deadUntil = now + DATAI(TEXT("Dead_Until_Time"),1000);
    IKASSERT(m_deadUntil>now);
    return;
  }
  if (m_bRepeatLatched)
    stage = kRepeat;

  if (stage == kRepeat && m_timeToRespond!=0)
    stage = kPress;

  //  do nothing if membrane is repeating but the key says no
  if (stage== kRepeat && HasNonRepeating(repeatDomainData))
  {
    stage = -1;

    //  lift any keys that are down.
    if (CountRealCodes(repeatDomainData)==1 && m_ndown!=0)
    {
      //  release this code
      if (!IsModifier(repeatDomainData[0]))
        PostKey(repeatDomainData[0],IK_UP);
      PostKeyDone();
      m_ndown = 0;
    }
  }

  //  do nothing if membrane is repeating but settings say no
  //  and we're NOT moving the mouse.
  if (stage== kRepeat && !pSettings->m_bRepeat && !HasOnlyMouseMoveActions(repeatDomainData))
    stage = -1;

  //  do nothing if the membrane is repeating but we have mouse clicks
  if (stage== kRepeat && HasMouseClicks(repeatDomainData))
    stage = -1;

  //  do nothing if the membrane is repeating but we have setup codes
  if (stage== kRepeat && HasSetupCodes(repeatDomainData))
    stage = -1;

  //  disallow slide with required lift off
  if (stage == kSlide && pSettings->m_bRequiredLiftOff)
    stage = -1;

  //  disallow repeat with required lift off if we're not where we started
  if (stage == kRepeat && pSettings->m_bRequiredLiftOff && newDomain != m_repeatDomain)
  {
    stage = -1;
    LiftAllModifiers();
    ResetKeyboard();
    ResetMouse();
    PostKeyDone();
    m_ndown = 0;
    m_repeatAfter = 0;
    m_repeatDomain = 0;
    m_timeToRespond = 0;
  }

  //  pressing or sliding: do the key
  if ( stage == kPress || stage == kSlide )
  {
    //  if sliding, make sure repeating was stopped.
    if (stage == kSlide)
    {
      if (CountRealCodes(currentDomainData)==1 && m_ndown!=0)
      {
        //  release this code
        if (!IsModifier(currentDomainData[0]))
          PostKey(currentDomainData[0],IK_UP);
        PostKeyDone();
        m_ndown = 0;
      }

      m_repeatAfter = 0;
      m_repeatDomain = 0;
      m_timeToRespond = 0;
    }


    {
      //  response rate

      if (now>m_timeToRespond)
      {
        int delay;
        if (switchNumber>0)
          delay = DATAI(TEXT("Switch_Response_Period"),50);
        else
          delay = 20*(15-pSettings->m_iResponseRate)*(15-pSettings->m_iResponseRate) + DATAI(TEXT("Min_Response_Time"),0);
        if(delay>0 && m_timeToRespond==0)
        {
          m_timeToRespond = now + delay;
        }
        else
        {
          m_timeToRespond = 0;

          if (IsIdle() && ((!pSettings->m_bRequiredLiftOff) ||
                           (pSettings->m_bRequiredLiftOff && m_lastRelease > m_lastExecute)))
          {
            (void) ComputeMouseInc ( 0 ); //  resets accell

            ShortKeySound();
            PostKeyStart();

            AppLibKillFloatingMessage();

            m_ndown = ExecuteUniversalData(newDomainData);
            m_lastExecute = now; // record time of most recent execution
          }
        }
      }
    }

    //  compute when to Start repeating
    //  anywhere from m_minRepeatStart to m_maxRepeatStart, depending on the repeat rate

    int mr1 = DATAI(TEXT("Min_Repeat_Start"),1000);
    int mr2 = DATAI(TEXT("Max_Repeat_Start"),4000);
    int waitTime = mr1 +
                   (15-pSettings->m_iRepeatRate)*(mr2-mr1)/14;
    m_repeatAfter = IKUtil::GetCurrentTimeMS() + waitTime;
    m_repeatDomain = newDomain;
    m_nrepeat = 0;
  }

    //  releasing - stop any repeating
  else if (stage == kRelease)  //   && !PressAnywhere)
  {
    if (CountRealCodes(currentDomainData)==1 && m_ndown!=0)
    {
      //  release this code 'cause it was not released
      //  before
      if (!IsModifier(currentDomainData[0]))
        PostKey(currentDomainData[0],IK_UP);
      m_ndown = 0;
    }

    if (IKSettings::GetSettings()->m_iShiftKeyAction==kSettingsShiftNoLatch)
      LiftAllModifiers();

    PurgeToLastStart();
    PostKeyDone();
    m_repeatAfter = 0;
    m_repeatDomain = 0;
    m_timeToRespond = 0;
  }

    //  repeating
  else if (stage == kRepeat && repeatDomainData!=NULL)
  {
    m_timeToRespond = 0;

    //  is it time to repeat yet?
    if (now>=m_repeatAfter && m_repeatAfter!=0)
    {
      //  run the key again
      PostKeyRepeat();
      m_ndown = ExecuteUniversalData(repeatDomainData);

      //  initialize some data for repeat latching
      if (pSettings->m_bRepeat && pSettings->m_bRepeatLatching)
      {
        m_nrepeat++;
        m_bRepeatLatched = true;
        if (m_nrepeat==1)
          m_bRepeatLatchReleased = false;
      }

      //  compute the next time to repeat.  If any mouse
      //  actions, do it right away.
      int delay = 0;
      if (!HasMouseActions(repeatDomainData))
        delay = GetRepeatDelayMS();
      m_repeatAfter = now + delay;
    }
    else
    {
      //  not time to repeat yet.
      //  But if it's a single mouse move, do it anyway.
      if (IsMouseMove(repeatDomainData[0]) && repeatDomainData[1]==0)
      {
        m_ndown = ExecuteUniversalData(repeatDomainData);
      }
    }
  }

  //  the end.

  m_currentDomain = newDomain;
#endif
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
  // implement later
  return false;
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
      IK_PRINTF("ProcessCommand: %s\r\n", ik_cmd_str[cmd_id]);
    }

    // blocking until report is sent
    while (!tuh_hid_send_report(_daddr, idx, 0, command, IK_REPORT_LEN)) {
      tuh_task();
    }

#if 0
    _expected_resp++;
    tuh_hid_receive_report(_daddr, idx);
#endif
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
  PostDelay(5);
  uint8_t command[IK_REPORT_LEN] = {
      IK_CMD_CP_REPORT_REALTIME, bForce, 0, 0, 0, 0, 0, 0};
  PostCommand(command);
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
    IK_PRINTF("Event: %s: ", ik_event_str[event_id - EVENT_BASE]);
    for (uint8_t i = 0; i < len; i++) {
      IK_PRINTF("%02x ", data[i]);
    }
    IK_PRINTF("\n");
  }
#endif

  switch (event_id) {
  case IK_EVENT_MEMBRANE_PRESS:
    OnMembranePress(data[1], data[2]);
    if (!HasStandardOverlay()) {
      InterpretRaw();
    }
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_MEMBRANE_RELEASE:
    OnMembraneRelease(data[1], data[2]);
    if (!HasStandardOverlay()) {
      InterpretRaw();
    }
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_SWITCH:
    OnSwitch(data[1], data[2]);
    if (!HasStandardOverlay()) {
      InterpretRaw();
    }
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
    if (!HasStandardOverlay()) {
      InterpretRaw();
    }
    PostReportDataToControlPanel();
    break;

  case IK_EVENT_CORRECT_SWITCH:
    OnCorrectSwitch(data[1]);
    if (!HasStandardOverlay()) {
      InterpretRaw();
    }
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
  m_lastExecuted = NULL;

  for (int i2 = 0; i2 < 5; i2++) {
    m_last5Overlays[i2] = -1;
  }

  //  reset keyboard
  for (unsigned int i = 0; i < sizeof(m_KeyBoardReport); i++) {
    m_KeyBoardReport[i] = 0;
  }

  uint8_t msg[IK_REPORT_LEN] = {IK_CMD_REFLECT_KEYSTROKE, 0, 0, 0, 0, 0, 0, 0};
  for (unsigned int j = 0; j < sizeof(m_KeyBoardReport); j++) {
    msg[j + 1] = m_KeyBoardReport[j];
  }

  PostCommand(msg);

  //  reconcile with modifier objects?
  PostLiftAllModifiers();
}

void Adafruit_IntelliKeys::ResetMouse(void) {
  //  reset mouse
  for (unsigned int i = 0; i < sizeof(m_MouseReport); i++) {
    m_MouseReport[i] = 0;
  }

  uint8_t msg2[] = {IK_CMD_REFLECT_MOUSE_MOVE, 0, 0, 0, 0, 0, 0, 0};
  for (unsigned int j = 0; j < sizeof(m_MouseReport); j++) {
    msg2[j + 1] = m_MouseReport[j];
  }

  PostCommand(msg2);
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
  if (GetDevType() != 1) {
    return false; //  only IK has standard overlays
  }

  return (m_currentOverlay != 7 && m_currentOverlay != -1);
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

#if 0
  if (_expected_resp) {
    _expected_resp--;
    if (!tuh_hid_receive_report(daddr, idx)) {
      IK_PRINTF("Failed to receive report\n");
      _expected_resp++;
      return;
    }
  }
#else
  if (!tuh_hid_receive_report(daddr, idx)) {
    IK_PRINTF("Failed to receive report\n");
    return;
  }
#endif
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
