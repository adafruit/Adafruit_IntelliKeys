// IKSettings.cpp: implementation of the IKSettings class.
//
//////////////////////////////////////////////////////////////////////

// #include "IKCommon.h"
#include "IKSettings.h"
// #include "IKFile.h"

// #include "IKUtil.h"

// #include <stdio.h>

#define TEXT(_x) (_x)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
////////////////////////////////////////////////////////////////////

static IKSettings settings;

IKSettings *IKSettings::GetSettings() { return &settings; }

// copy ctor

IKSettings::IKSettings()
    :

      m_iResponseRate(kSettingsRateHigh),

      m_bRequiredLiftOff(false),

      m_bUseSystemRepeatSettings(true),

      m_iKeySoundVolume(kSettingsKeysound2),

      m_iRepeatRate(kSettingsRateHigh),

      m_bRepeat(true),

      m_bShowModeWarning(true),

      m_bRepeatLatching(false),

      m_iShiftKeyAction(kSettingsShiftLatching),

      m_iMouseSpeed(kSettingsRateHigh),

      m_bSmartTyping(false),

      m_iDataSendRate(kSettingsRateHigh),

      m_iMakeBreakRate(kSettingsRateHigh),

      m_iIndicatorLights(kSettings6lights),

      m_iMode(kSettingsModeLastSentOverlay),

      m_iUseThisSwitchSetting(0),

      // m_sUseThisOverlay			(TEXT(""))

      m_sLastSent(TEXT("")), m_sLastSentBy(TEXT("")),

      m_bButAllowOverlays(true)

{
  SetToDefault();
}

IKSettings::~IKSettings() {}

IKSettings &IKSettings ::operator=(const IKSettings &rhs) {
  m_iResponseRate = rhs.m_iResponseRate;
  m_bRequiredLiftOff = rhs.m_bRequiredLiftOff;
  m_bUseSystemRepeatSettings = rhs.m_bUseSystemRepeatSettings;
  m_iKeySoundVolume = rhs.m_iKeySoundVolume;
  m_iRepeatRate = rhs.m_iRepeatRate;
  m_bRepeat = rhs.m_bRepeat;
  m_bRepeatLatching = rhs.m_bRepeatLatching;
  m_iShiftKeyAction = rhs.m_iShiftKeyAction;
  m_iMouseSpeed = rhs.m_iMouseSpeed;
  m_bSmartTyping = rhs.m_bSmartTyping;
  m_iDataSendRate = rhs.m_iDataSendRate;
  m_iMakeBreakRate = rhs.m_iMakeBreakRate;
  m_iIndicatorLights = rhs.m_iIndicatorLights;
  // m_group					= rhs.m_group;
  // m_student				= rhs.m_student;
  m_iMode = rhs.m_iMode;
  m_iUseThisSwitchSetting = rhs.m_iUseThisSwitchSetting;
  m_sUseThisOverlay = rhs.m_sUseThisOverlay;

  m_keyMap = rhs.m_keyMap;

  m_sLastSent = rhs.m_sLastSent;
  m_sLastSentBy = rhs.m_sLastSentBy;

  m_bShowModeWarning = rhs.m_bShowModeWarning;

  m_bButAllowOverlays = rhs.m_bButAllowOverlays;

  return *this;
}

bool IKSettings ::operator!=(const IKSettings &rhs) { return !(*this == rhs); }

bool IKSettings ::operator==(const IKSettings &rhs) {
  return (m_iResponseRate == rhs.m_iResponseRate) &&
         (m_bRequiredLiftOff == rhs.m_bRequiredLiftOff) &&
         (m_bUseSystemRepeatSettings == rhs.m_bUseSystemRepeatSettings) &&
         (m_iKeySoundVolume == rhs.m_iKeySoundVolume) &&
         (m_iRepeatRate == rhs.m_iRepeatRate) && (m_bRepeat == rhs.m_bRepeat) &&
         (m_bRepeatLatching == rhs.m_bRepeatLatching) &&
         (m_iShiftKeyAction == rhs.m_iShiftKeyAction) &&
         (m_iMouseSpeed == rhs.m_iMouseSpeed) &&
         (m_bSmartTyping == rhs.m_bSmartTyping) &&
         (m_iDataSendRate == rhs.m_iDataSendRate) &&
         (m_iMakeBreakRate == rhs.m_iMakeBreakRate) &&
         (m_iIndicatorLights == rhs.m_iIndicatorLights) &&
         //( m_group				== rhs.m_group
         //)
         //&&
         //( m_student				== rhs.m_student
         //)
         //&&
         (m_iMode == rhs.m_iMode) &&
         (m_iUseThisSwitchSetting == rhs.m_iUseThisSwitchSetting) &&
         (m_sUseThisOverlay == rhs.m_sUseThisOverlay) &&
         (m_sLastSent == rhs.m_sLastSent) &&
         (m_sLastSentBy == rhs.m_sLastSentBy) &&
         (m_bShowModeWarning == rhs.m_bShowModeWarning) &&
         (m_bButAllowOverlays == rhs.m_bButAllowOverlays) && true;
}

bool IKSettings::Read(IKString filename) {
  //  set to defaults first
  SetToDefault();

#if 0
	//  load the file
  bool bLoaded = m_keyMap.Read(filename);
#else
  bool bLoaded = false;
#endif

  // extract loaded values

  if (bLoaded) {
    m_iResponseRate = GetIntValue(TEXT("Response Rate"));
    m_iRepeatRate = GetIntValue(TEXT("Repeat Rate"));
    m_iShiftKeyAction = GetIntValue(TEXT("Shift Key Action"));
    m_iMouseSpeed = GetIntValue(TEXT("Mouse Speed"));
    m_iDataSendRate = GetIntValue(TEXT("Data Send Rate"));
    m_iMakeBreakRate = GetIntValue(TEXT("Make Break Rate"));
    m_iIndicatorLights =
        kSettings6lights; // GetIntValue(TEXT("Indicator Lights"));

    m_bRequiredLiftOff = GetBoolValue(TEXT("Required Lift Off"));
    m_bUseSystemRepeatSettings =
        GetBoolValue(TEXT("Use System Repeat Settings"));
    m_iKeySoundVolume = GetIntValue(TEXT("Key Sound Volume"));
    m_bRepeat = GetBoolValue(TEXT("Repeat"));
    m_bRepeatLatching = GetBoolValue(TEXT("Repeat Latching"));
    m_bSmartTyping = GetBoolValue(TEXT("Smart Typing"));

    m_iMode = GetIntValue(TEXT("Mode"));
    m_iUseThisSwitchSetting = GetIntValue(TEXT("Use This Switch Setting"));
    m_sUseThisOverlay = GetStringValue(TEXT("Use This Overlay"));
    m_sLastSent = GetStringValue(TEXT("Last Sent"));
    m_sLastSentBy = GetStringValue(TEXT("Last Sent By"));

    m_bShowModeWarning = GetBoolValue(TEXT("Show Mode Warning"));
    m_bButAllowOverlays = GetBoolValue(TEXT("But Allow Overlays"));
  }

  return true;
}

void IKSettings::Write(IKString filename) {
  StoreValues();

  //  save the file
#if 0
  m_keyMap.Write(filename);
#endif
}

void IKSettings::Write() {
  StoreValues();

  //  save the file
#if 0
  m_keyMap.Write();
#endif
}

int IKSettings::GetIntValue(TCHAR *pKey) {
#if 0
	IKString str = GetStringValue(pKey);
  return IKUtil::StringToInt(str);
#else
  return 0;
#endif
}

void IKSettings::SetIntValue(TCHAR *pKey, int value) {
#if 0
  m_keyMap.SetValue(pKey,value);
#endif
}

bool IKSettings::GetBoolValue(TCHAR *pKey) {
#if 0
	IKString str = GetStringValue(pKey);
	if(str.Compare(TEXT("true"))==0)

		return true;

	if(str.Compare(TEXT("TRUE"))==0)

		return true;

#endif
  return false;
}

void IKSettings::SetBoolValue(TCHAR *pKey, bool bValue) {
#if 0
  if (bValue)
		SetStringValue ( pKey, TEXT("TRUE"));
	else
		SetStringValue ( pKey, TEXT("FALSE"));
#endif
}

IKString IKSettings::GetStringValue(TCHAR *pKey) {
#if 0
  IKString str = m_keyMap.GetValueString(pKey);
	return str;
#else
  return NULL;
#endif
}

void IKSettings::SetStringValue(TCHAR *pKey, TCHAR *pValue) {
#if 0
  m_keyMap.Add(pKey,pValue);
#endif
}

void IKSettings::SetToDefault(bool bFeatureReset /*=false*/) {
  m_iResponseRate = kSettingsRateHigh;
  m_bRequiredLiftOff = false;
  m_bUseSystemRepeatSettings = true;
  m_iKeySoundVolume = kSettingsKeysound2;
  m_iRepeatRate = kSettingsRateHigh;
  m_bRepeat = true;
  m_bRepeatLatching = false;
  m_iShiftKeyAction = kSettingsShiftLatching;
  m_iMouseSpeed = kSettingsRateHigh;
  m_bSmartTyping = false;
  m_iDataSendRate = kSettingsRateHigh;
  m_iMakeBreakRate = kSettingsRateHigh;
  m_iIndicatorLights = kSettings6lights;
  m_iMode = kSettingsModeLastSentOverlay;
  m_iUseThisSwitchSetting = 0;
  m_sUseThisOverlay = TEXT("");
  m_bShowModeWarning = true;
  m_bButAllowOverlays = true;

  if (!bFeatureReset) {
    m_sLastSent = TEXT("");
    m_sLastSentBy = TEXT("");
  }
}

void IKSettings::StoreValues() {
  //  store new values
#if 0
	SetIntValue  ( TEXT("Repeat Rate"),					m_iRepeatRate);
	SetIntValue  ( TEXT("Shift Key Action"),			m_iShiftKeyAction);
	SetIntValue  ( TEXT("Response Rate"),				m_iResponseRate);
	SetIntValue  ( TEXT("Mouse Speed"),					m_iMouseSpeed);
	SetIntValue  ( TEXT("Data Send Rate"),				m_iDataSendRate);
	SetIntValue  ( TEXT("Make Break Rate"),				m_iMakeBreakRate);
	SetIntValue  ( TEXT("Indicator Lights"),			m_iIndicatorLights);
	SetIntValue  ( TEXT("Make Break Rate"),				m_iDataSendRate);

	SetBoolValue ( TEXT("Required Lift Off"),			m_bRequiredLiftOff);
	SetBoolValue ( TEXT("Use System Repeat Settings"),	m_bUseSystemRepeatSettings);
	SetIntValue  ( TEXT("Key Sound Volume"),			m_iKeySoundVolume);
	SetBoolValue ( TEXT("Repeat"),						m_bRepeat);
	SetBoolValue ( TEXT("Repeat Latching"),				m_bRepeatLatching);
	SetBoolValue ( TEXT("Smart Typing"),				m_bSmartTyping);

	SetIntValue    ( TEXT("Mode"), m_iMode );
	SetIntValue    ( TEXT("Use This Switch Setting"),	m_iUseThisSwitchSetting );
	SetStringValue ( TEXT("Use This Overlay"),			m_sUseThisOverlay );
	SetStringValue ( TEXT("Last Sent"),					m_sLastSent );
	SetStringValue ( TEXT("Last Sent By"),				m_sLastSentBy );

	SetBoolValue   ( TEXT("Show Mode Warning"),			m_bShowModeWarning );
	SetBoolValue   ( TEXT("But Allow Overlays"),		m_bButAllowOverlays );
#endif
}

//////////////////////////////////
//
//  copy ctor

IKSettings::IKSettings(const IKSettings &src) {
  m_iResponseRate = src.m_iResponseRate;
  m_bRequiredLiftOff = src.m_bRequiredLiftOff;
  m_bUseSystemRepeatSettings = src.m_bUseSystemRepeatSettings;
  m_iKeySoundVolume = src.m_iKeySoundVolume;
  m_iRepeatRate = src.m_iRepeatRate;
  m_bRepeat = src.m_bRepeat;
  m_bRepeatLatching = src.m_bRepeatLatching;
  m_iShiftKeyAction = src.m_iShiftKeyAction;
  m_iMouseSpeed = src.m_iMouseSpeed;
  m_bSmartTyping = src.m_bSmartTyping;
  m_iDataSendRate = src.m_iDataSendRate;
  m_iMakeBreakRate = src.m_iMakeBreakRate;
  m_iIndicatorLights = src.m_iIndicatorLights;
  // m_group					= rhs.m_group;
  // m_student				= rhs.m_student;
  m_iMode = src.m_iMode;
  m_iUseThisSwitchSetting = src.m_iUseThisSwitchSetting;
  m_sUseThisOverlay = src.m_sUseThisOverlay;

  m_keyMap = src.m_keyMap;

  m_sLastSent = src.m_sLastSent;
  m_sLastSentBy = src.m_sLastSentBy;

  m_bShowModeWarning = src.m_bShowModeWarning;
  m_bButAllowOverlays = src.m_bButAllowOverlays;
}
