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

#include "IKOverlay.h"
#include "class/hid/hid.h"

#define IK_DEBUG 0

#if IK_DEBUG
#include <Arduino.h>
#define IK_PRINTF(...) serial1_printf(__VA_ARGS__)
#else
#define IK_PRINTF(...)
#endif

IKOverlay stdOverlays[7];

IKOverlay::IKOverlay() { memset(_membrane, 0, sizeof(_membrane)); }

void IKOverlay::getSwitchReport(int nswitch, ik_report_t *report) {}

void IKOverlay::getMembraneReport(int row, int col, ik_report_t *report) {
  if (row > IK_RESOLUTION_X || col > IK_RESOLUTION_Y) {
    return;
  }
  *report = _membrane[row][col];
}

void IKOverlay::setMembraneReport(int top_row, int top_col, int height,
                                  int width, ik_report_t *report) {
  if (!(top_row < IK_RESOLUTION_X && top_col < IK_RESOLUTION_Y)) {
    IK_PRINTF("Invalid top row or top col [%u, %u]\r\n", top_row, top_col);
    return;
  }

  if (!((top_row + height <= IK_RESOLUTION_X) &&
        (top_col + width <= IK_RESOLUTION_Y))) {
    IK_PRINTF("Invalid height or width [%u, %u] + [%u, %u] = [%u, %u]\r\n",
              top_row, top_col, height, width, top_row + height,
              top_col + width);
    return;
  }

  for (int row = top_row; row < top_row + height; row++) {
    for (int col = top_col; col < top_col + width; col++) {
      _membrane[row][col] = *report;
    }
  }
}

void IKOverlay::initStandardOverlays(void) {
  initStdMathAccess();
  initStdAlphabet();
  initStdBasicWriting();
}

void IKOverlay::initStdMathAccess(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_MATH_ACCESS];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int row, col, height, width;

  //------------- Calculator -------------//
  height = 6;
  width = 4;

  // row 1
  row = 0;
  col = 0;

  report.keyboard.keycode = HID_KEY_7;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_8;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_9;
  overlay.setMembraneReport(row, col, height, width, &report);

  // row 2
  row = 6;
  col = 0;

  report.keyboard.keycode = HID_KEY_4;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_5;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_6;
  overlay.setMembraneReport(row, col, height, width, &report);

  // row 3
  row = 12;
  col = 0;

  report.keyboard.keycode = HID_KEY_1;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_2;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_3;
  overlay.setMembraneReport(row, col, height, width, &report);

  // row 4
  row = 18;
  col = 0;

  report.keyboard.keycode = HID_KEY_BACKSPACE;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_0;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_ENTER;
  overlay.setMembraneReport(row, col, height, width, &report);

  //------------- Left Pad -------------//
  height = 3;
  width = 2;

  // row 1
  row = 0;
  col = 14;

  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = HID_KEY_EQUAL;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_MINUS;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = HID_KEY_4;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_ARROW_LEFT;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_ARROW_RIGHT;
  overlay.setMembraneReport(row, col, height, width, &report);

  // row 2
  row = 3;
  col = 14;

  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = HID_KEY_8;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_SLASH;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_PERIOD;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_ARROW_UP;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_ARROW_DOWN;
  overlay.setMembraneReport(row, col, height, width, &report);

  // row 3
  row = 6;
  col = 14;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_EQUAL;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  col += 2 * width;
  report.keyboard.keycode = HID_KEY_SPACE;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  col += 2 * width;
  report.keyboard.keycode = HID_KEY_TAB;
  overlay.setMembraneReport(row, col, height, width, &report);

  // row 4
  row = 9;
  col = 14;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_BACKSPACE; // TODO: change to CLEAR
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  col += 2 * width;
  report.keyboard.keycode = HID_KEY_ESCAPE;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += 2 * width;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = 0;

  // TODO row 5 to 8 is Mouse
}

void IKOverlay::initStdBasicWriting(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_BASIC_WRITING];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  int row, col;

  // for most keys, height = 3, width = 2
  int const height = 3;
  int const width = 2;

  //------------- First Row -------------//
  // mouse only

  //------------- Second Row -------------//
  row = 3;
  col = 0;

  report.keyboard.keycode = HID_KEY_ESCAPE;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_TAB;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTALT;
  report.keyboard.keycode = 0;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTGUI;
  report.keyboard.keycode = 0;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTCTRL;
  report.keyboard.keycode = 0;
  overlay.setMembraneReport(row, col, height, width, &report);

  // TODO more mouse

  //------------- Third Row -------------//
  row = 6;
  col = 0;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_PERIOD;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_COMMA;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_APOSTROPHE;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_1;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_SLASH;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_MINUS;
  overlay.setMembraneReport(row, col, height, width, &report);

  // TODO more mouse

  //------------- Fourth Row -------------//
  row = 9;
  col = 0;

  report.keyboard.modifier = 0;
  for (int i = 0; i < 10; i++) {
    report.keyboard.keycode = HID_KEY_1 + i;
    overlay.setMembraneReport(row, col, height, width, &report);

    col += width;
  }

  report.keyboard.keycode = HID_KEY_BACKSPACE;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Fifth Row -------------//
  row = 12;
  col = 3; // first key is empty

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_Q;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_W;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_E;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_R;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_T;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_Y;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_U;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_I;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_O;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_P;
  overlay.setMembraneReport(row, col, height, width, &report);

  //------------- Sixth Row -------------//
  row = 15;
  col = 0;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_CAPS_LOCK;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_A;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_S;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_D;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_F;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_G;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_H;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_J;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_K;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_L;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_ENTER;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Seventh Row -------------//
  row = 18;
  col = 0;

  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = 0;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  col += 2 * width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_Z;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_X;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_C;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_V;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_B;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_N;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_M;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.modifier = KEYBOARD_MODIFIER_RIGHTSHIFT;
  report.keyboard.keycode = 0;
  overlay.setMembraneReport(row, col, height, 2 * width, &report);

  //------------- Eighth Row -------------//
  row = 21;
  col = 8;

  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_SPACE;
  overlay.setMembraneReport(row, col, height, 3 * width, &report);

  col += 4 * width;
  report.keyboard.modifier = 0;
  report.keyboard.keycode = HID_KEY_ARROW_LEFT;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_ARROW_RIGHT;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_ARROW_UP;
  overlay.setMembraneReport(row, col, height, width, &report);

  col += width;
  report.keyboard.keycode = HID_KEY_ARROW_DOWN;
  overlay.setMembraneReport(row, col, height, width, &report);
}

// Alphabet Overlay
void IKOverlay::initStdAlphabet(void) {
  IKOverlay &overlay = stdOverlays[IK_OVERLAY_ALPHABET];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  //------------- First 2 Rows -------------//
  report.keyboard.keycode = HID_KEY_ESCAPE;
  overlay.setMembraneReport(0, 0, 4, 4, &report);

  report.keyboard.keycode = HID_KEY_CAPS_LOCK;
  overlay.setMembraneReport(0, 4, 4, 4, &report);

  report.keyboard.keycode = HID_KEY_BACKSPACE;
  overlay.setMembraneReport(0, 8, 4, 4, &report);

  report.keyboard.keycode = HID_KEY_ARROW_LEFT;
  overlay.setMembraneReport(1, 14, 4, 3, &report);

  report.keyboard.keycode = HID_KEY_ARROW_UP;
  overlay.setMembraneReport(0, 18, 4, 3, &report);

  report.keyboard.keycode = HID_KEY_ARROW_RIGHT;
  overlay.setMembraneReport(1, 21, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_ARROW_DOWN;
  overlay.setMembraneReport(4, 18, 4, 3, &report);

  report.keyboard.keycode = HID_KEY_PERIOD;
  overlay.setMembraneReport(4, 1, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_COMMA;
  overlay.setMembraneReport(4, 3, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_SLASH;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  overlay.setMembraneReport(4, 5, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_1;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  overlay.setMembraneReport(4, 7, 4, 2, &report);

  report.keyboard.modifier = 0;

  //------------- a -> h -------------//
  for (int i = 0; i < 8; i++) {
    int const row = 8;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_A + i;
    overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  //------------- i -> p -------------//
  for (int i = 0; i < 8; i++) {
    int const row = 12;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_I + i;
    overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  //------------- q -> v, enter -------------//
  for (int i = 0; i < 6; i++) {
    int const row = 16;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_Q + i;
    overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  report.keyboard.keycode = HID_KEY_ENTER;
  overlay.setMembraneReport(16, 18, 4, 6, &report);

  //------------- shift, w -> z, shift, space -------------//
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = 0;

  overlay.setMembraneReport(20, 0, 4, 3, &report);

  report.keyboard.modifier = KEYBOARD_MODIFIER_RIGHTSHIFT;
  overlay.setMembraneReport(20, 15, 4, 3, &report);

  report.keyboard.modifier = 0;
  for (int i = 1; i < 5; i++) {
    int const row = 20;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_W + i - 1;
    overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  report.keyboard.keycode = HID_KEY_SPACE;
  overlay.setMembraneReport(20, 18, 3, 6, &report);
}
