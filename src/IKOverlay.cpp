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
  IKOverlay &abc_overlay = stdOverlays[IK_OVERLAY_ALPHABET];

  ik_report_t report;
  report.type = IK_REPORT_TYPE_KEYBOARD;
  report.keyboard.modifier = 0;

  //--------------------------------------------------------------------+
  // Alphabet Overlay
  //--------------------------------------------------------------------+

  //------------- First 2 Rows -------------//
  report.keyboard.keycode = HID_KEY_ESCAPE;
  abc_overlay.setMembraneReport(0, 0, 4, 4, &report);

  report.keyboard.keycode = HID_KEY_CAPS_LOCK;
  abc_overlay.setMembraneReport(0, 4, 4, 4, &report);

  report.keyboard.keycode = HID_KEY_BACKSPACE;
  abc_overlay.setMembraneReport(0, 8, 4, 4, &report);

  report.keyboard.keycode = HID_KEY_ARROW_LEFT;
  abc_overlay.setMembraneReport(1, 14, 4, 3, &report);

  report.keyboard.keycode = HID_KEY_ARROW_UP;
  abc_overlay.setMembraneReport(0, 18, 4, 3, &report);

  report.keyboard.keycode = HID_KEY_ARROW_RIGHT;
  abc_overlay.setMembraneReport(1, 21, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_ARROW_DOWN;
  abc_overlay.setMembraneReport(4, 18, 4, 3, &report);

  report.keyboard.keycode = HID_KEY_PERIOD;
  abc_overlay.setMembraneReport(4, 1, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_COMMA;
  abc_overlay.setMembraneReport(4, 3, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_SLASH;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  abc_overlay.setMembraneReport(4, 5, 4, 2, &report);

  report.keyboard.keycode = HID_KEY_1;
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  abc_overlay.setMembraneReport(4, 7, 4, 2, &report);

  report.keyboard.modifier = 0;

  //------------- a -> h -------------//
  for (int i = 0; i < 8; i++) {
    int const row = 8;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_A + i;
    abc_overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  //------------- i -> p -------------//
  for (int i = 0; i < 8; i++) {
    int const row = 12;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_I + i;
    abc_overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  //------------- q -> v, enter -------------//
  for (int i = 0; i < 6; i++) {
    int const row = 16;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_Q + i;
    abc_overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  report.keyboard.keycode = HID_KEY_ENTER;
  abc_overlay.setMembraneReport(16, 18, 4, 6, &report);

  //------------- shift, w -> z, shift, space -------------//
  report.keyboard.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
  report.keyboard.keycode = 0;

  abc_overlay.setMembraneReport(20, 0, 4, 3, &report);

  report.keyboard.modifier = KEYBOARD_MODIFIER_RIGHTSHIFT;
  abc_overlay.setMembraneReport(20, 15, 4, 3, &report);

  report.keyboard.modifier = 0;
  for (int i = 1; i < 5; i++) {
    int const row = 20;
    int col = i * 3;
    report.keyboard.keycode = HID_KEY_W + i - 1;
    abc_overlay.setMembraneReport(row, col, 4, 3, &report);
  }

  report.keyboard.keycode = HID_KEY_SPACE;
  abc_overlay.setMembraneReport(20, 18, 3, 6, &report);
}
