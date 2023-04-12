/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Ha Thach (thach@tinyusb.org) for Adafruit Industries
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

#ifndef ADAFRUIT_INTELLIKEYS_IKOVERLAY_H
#define ADAFRUIT_INTELLIKEYS_IKOVERLAY_H

#include "intellikeysdefs.h"

/* The standard overlays
Standard_Overlay_0_Name		Web Access USB Overlay
Standard_Overlay_1_Name		Setup USB Overlay
Standard_Overlay_2_Name		Math Access USB Overlay
Standard_Overlay_3_Name		Alphabet USB Overlay
Standard_Overlay_4_Name		Mouse Access USB Overlay
Standard_Overlay_5_Name		QWERTY USB Overlay
Standard_Overlay_6_Name		Basic Writing USB Overlay
Standard_Overlay_7_Name		none
*/

#define IK_OVERLAY_WEB_ACCESS 0
#define IK_OVERLAY_SETUP 1
#define IK_OVERLAY_MATH_ACCESS 2
#define IK_OVERLAY_ALPHABET 3
#define IK_OVERLAY_MOUSE_ACCESS 4
#define IK_OVERLAY_QWERTY 5
#define IK_OVERLAY_BASIC_WRITING 6

enum { IK_REPORT_TYPE_NONE = 0, IK_REPORT_TYPE_KEYBOARD, IK_REPORT_TYPE_MOUSE };

typedef struct __attribute__((packed)) {
  uint8_t type; // 0: for none, 1 for keyboard, 2 for mouse
  union {
    struct {
      uint8_t buttons;
      int8_t x;
      int8_t y;
    } mouse;

    struct {
      uint8_t modifier;
      uint8_t keycode;
    } keyboard;
  };
} ik_report_t;

class IKOverlay {
public:
  IKOverlay();
  static void initStdMathAccess(void);
  static void initStdAlphabet(void);
  static void initStdQwerty(void);
  static void initStdBasicWriting(void);

  static void initStandardOverlays(void);

  void setMembraneReport(int top_row, int top_col, int height, int width,
                         ik_report_t *report);

  void getSwitchReport(int nswitch, ik_report_t *report);
  void getMembraneReport(int row, int col, ik_report_t *report);

  void setMembraneKeyboardRow(int row, int col, int height, int width,
                              uint8_t const kbd_item[][2], uint8_t count);

private:
  ik_report_t _membrane[IK_RESOLUTION_X][IK_RESOLUTION_Y];

  // init row QWERTY
  void initQwertyRow(int row, int col, int height, int width);
  void initAsdfghRow(int row, int col, int height, int width);
  void initZxcvbnRow(int row, int col, int height, int width);
};

extern IKOverlay stdOverlays[7];

#endif // ADAFRUIT_INTELLIKEYS_IKOVERLAY_H
