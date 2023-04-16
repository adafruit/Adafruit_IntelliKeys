/*********************************************************************
 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 Copyright (c) 2019 Ha Thach for Adafruit Industries
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

/* This example demonstrates use of both device and host, where
 * - Device run on native usb controller (controller0)
 * - Host run on bit-banging 2 GPIOs with the help of Pico-PIO-USB library
 * (controller1)
 *
 * Example sketch receive keyboard report from host interface (from e.g consumer
 * keyboard) and remap it to another key and send it via device interface (to
 * PC). For simplicity, this example only toggle shift key to the report,
 * effectively remap:
 * - all character key <-> upper case
 * - number <-> its symbol (with shift)
 *
 * Requirements:
 * - [Pico-PIO-USB](https://github.com/sekigon-gonnoc/Pico-PIO-USB) library
 * - 2 consecutive GPIOs: D+ is defined by PIN_USB_HOST_DP, D- = D+ +1
 * - Provide VBus (5v) and GND for peripheral
 * - CPU Speed must be either 120 or 240 Mhz. Selected via "Menu -> CPU Speed"
 */

// pio-usb is required for rp2040 host
#include "pio_usb.h"

#include "Adafruit_TinyUSB.h"

#include "Adafruit_IntelliKeys.h"

// use rp2040 tester board
// #define USE_RP2040_TESTER

#ifdef USE_RP2040_TESTER

#define PIN_USB_HOST_DP 20
#define PIN_5V_EN 22
#define PIN_NEOPIXEL 13
#define NEOPIXEL_POWER 28
#define PIN_5V_EN_STATE 1

#else

// Pin D+ for host, D- = D+ + 1
#ifndef PIN_USB_HOST_DP
#define PIN_USB_HOST_DP 16 // 20
#endif

// Pin for enabling Host VBUS. comment out if not used
#ifndef PIN_5V_EN
#define PIN_5V_EN 18 // 22
#endif

#ifndef PIN_5V_EN_STATE
#define PIN_5V_EN_STATE 1
#endif

#ifndef PIN_NEOPIXEL
#define PIN_NEOPIXEL 21
#endif

#ifndef NEOPIXEL_POWER
#define NEOPIXEL_POWER 20
#endif

#endif

#define SCAN_INTERVAL 8

// USB Host object
Adafruit_USBH_Host USBHost;

Adafruit_IntelliKeys IKeys;

// HID report descriptor for keyboard and mouse
// Single Report (no ID) descriptor
uint8_t const desc_keyboard_report[] = {TUD_HID_REPORT_DESC_KEYBOARD()};
uint8_t const desc_mouse_report[] = {TUD_HID_REPORT_DESC_MOUSE()};

// USB HID object. For ESP32 these values cannot be changed after this
// declaration desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_keyboard(desc_keyboard_report,
                               sizeof(desc_keyboard_report),
                               HID_ITF_PROTOCOL_KEYBOARD, 8, false);

Adafruit_USBD_HID usb_mouse(desc_mouse_report, sizeof(desc_mouse_report),
                            HID_ITF_PROTOCOL_MOUSE, 8, false);

//------------- prototypes -------------//
enum { PIXEL_BRIGHTNESS = 0x20 };

enum {
  COLOR_NO_USB = 0xff0000,
  COLOR_NOT_READY = 0xffff00,
  COLOR_READY = 0x00ff00,
  COLOR_KEY_PRESSED = 0x0000ff
};

void setPixel(uint32_t color);

//--------------------------------------------------------------------+
// Setup and Loop on Core0
//--------------------------------------------------------------------+

void setup() {
  Serial.begin(115200);
  usb_keyboard.begin();
  usb_mouse.begin();

  // Enable neopixel power
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIXEL_POWER, HIGH);

  pinMode(PIN_NEOPIXEL, OUTPUT);
  digitalWrite(PIN_NEOPIXEL, LOW);
  setPixel(0);
  delay(1);

  setPixel(COLOR_NO_USB);

  // while ( !Serial ) delay(10);   // wait for native usb
  Serial.println("IntelliKeys USB Adapter");
}

bool hasKeyboardReport(hid_keyboard_report_t const *report) {
  if (report->modifier != 0) {
    return true;
  }

  for (uint8_t i = 0; i < 6; i++) {
    if (report->keycode[i] != 0) {
      return true;
    }
  }

  return false;
}

void scanMembraneAndSwitch(void) {
  static hid_keyboard_report_t kb_prev_report = {0, 0, {0}};
  static bool kb_has_prev_report = false;
  static uint8_t mouse_prev_buttons = 0;

  if (!IKeys.isAttached()) {
    setPixel(COLOR_NO_USB);
    return;
  }

  if (!IKeys.IsOpen() || !IKeys.IsSwitchedOn()) {
    setPixel(COLOR_NOT_READY);
    return;
  }

  uint32_t color = COLOR_READY;

  hid_keyboard_report_t kb_report;
  hid_mouse_report_t mouse_report;

  IKeys.getHIDReport(&kb_report, &mouse_report);

  //------------- Keyboard -------------//
  bool new_kb_report = hasKeyboardReport(&kb_report);

  if (new_kb_report) {
    if (memcmp(&kb_prev_report, &kb_report, sizeof(kb_report))) {
      // send only if kb_report is changed since last time
      usb_keyboard.sendReport(0, &kb_report, sizeof(kb_report));
    }
    kb_has_prev_report = true;
    color = COLOR_KEY_PRESSED;
  } else {
    if (kb_has_prev_report) {
      // has previous report before, send empty kb_report to release all keys
      hid_keyboard_report_t null_report = {0, 0, {0}};
      usb_keyboard.sendReport(0, &null_report, sizeof(null_report));
    }
    kb_has_prev_report = false;
  }

  kb_prev_report = kb_report;

  //------------- Mouse -------------//
  if (mouse_report.buttons != mouse_prev_buttons || mouse_report.x != 0 ||
      mouse_report.y != 0) {
    // x,y is only 0, 1, -1
    enum { MOUSE_SCALE = 1 };
    mouse_report.x *= MOUSE_SCALE;
    mouse_report.y *= MOUSE_SCALE;

    // TODO check for IK_REPORT_MOUSE_DOUBLE_CLICK and
    // IK_REPORT_MOUSE_CLICK_HOLD
    usb_mouse.sendReport(0, &mouse_report, sizeof(mouse_report));
    color = COLOR_KEY_PRESSED;
  }
  mouse_prev_buttons = mouse_report.buttons;

  setPixel(color);
}

void loop() {
  static uint32_t ms = 0;

  // scan every 10 ms
  if (millis() - ms > SCAN_INTERVAL) {
    ms = millis();
    scanMembraneAndSwitch();
  }

  Serial.flush();
}

//--------------------------------------------------------------------+
// Setup and Loop on Core1
//--------------------------------------------------------------------+

void setup1() {
  IKeys.begin();

  //  while (!Serial) {
  //    delay(10); // wait for native usb
  //  }
  Serial.println("Core1 setup to run TinyUSB host with pio-usb");

  // Check for CPU frequency, must be multiple of 120Mhz for bit-banging USB
  uint32_t cpu_hz = clock_get_hz(clk_sys);
  if (cpu_hz != 120000000UL && cpu_hz != 240000000UL) {
    while (!Serial)
      delay(10); // wait for native usb
    Serial.printf("Error: CPU Clock = %lu, PIO USB require CPU clock must be "
                  "multiple of 120 Mhz\r\n",
                  cpu_hz);
    Serial.printf("Change your CPU Clock to either 120 or 240 Mhz in Menu->CPU "
                  "Speed \r\n");
    while (1) {
      delay(1);
    }
  }

#ifdef PIN_5V_EN
  pinMode(PIN_5V_EN, OUTPUT);
  digitalWrite(PIN_5V_EN, PIN_5V_EN_STATE);
#endif

  pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  pio_cfg.pin_dp = PIN_USB_HOST_DP;
  USBHost.configure_pio_usb(1, &pio_cfg);

  // run host stack on controller (rhport) 1
  // Note: For rp2040 pico-pio-usb, calling USBHost.begin() on core1 will have
  // most of the host bit-banging processing works done in core1 to free up
  // core0 for other works
  USBHost.begin(1);
}

void loop1() {
  IKeys.Periodic();
  USBHost.task();
}

//--------------------------------------------------------------------+
// TinyUSB Host callbacks
// Note: running in the same core where Brain.USBHost.task() is called
//--------------------------------------------------------------------+
extern "C" {

void tuh_mount_cb(uint8_t daddr) {
  Serial.printf("Device attached, address = %d\r\n", daddr);
  IKeys.mount(daddr);
}

/// Invoked when device is unmounted (bus reset/unplugged)
void tuh_umount_cb(uint8_t daddr) {
  Serial.printf("Device detached, address = %d\r\n", daddr);
  IKeys.umount(daddr);
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const *report, uint16_t len) {
  IKeys.hid_reprot_received_cb(dev_addr, instance, report, len);
}

} // extern C

//--------------------------------------------------------------------+
// Neopixel bit-banging since PIO is used by USBH
//--------------------------------------------------------------------+

void __no_inline_not_in_flash_func(setPixel)(uint32_t color) {
  static uint32_t end_us = 0;
  static uint32_t last_color = 0x123456;

  if (last_color == color) {
    // no need to update
    return;
  }
  last_color = color;

  uint8_t r = (uint8_t)(color >> 16); // red
  uint8_t g = (uint8_t)(color >> 8);  // green
  uint8_t b = (uint8_t)color;

  // brightness correction
  r = (uint8_t)((r * PIXEL_BRIGHTNESS) >> 8);
  g = (uint8_t)((g * PIXEL_BRIGHTNESS) >> 8);
  b = (uint8_t)((b * PIXEL_BRIGHTNESS) >> 8);

  uint8_t buf[3] = {g, r, b};

  uint8_t *ptr, *end, p, bitMask;

  enum { PIN_MASK = (1ul << PIN_NEOPIXEL) };

  ptr = buf;
  end = ptr + 3;
  p = *ptr++;
  bitMask = 0x80;

  // wait for previous frame to finish
  enum { FRAME_TIME_US = 300 };

  uint32_t now = end_us;
  while (now - end_us < FRAME_TIME_US) {
    now = micros();
    if (now < end_us) {
      // micros() overflow
      end_us = now;
    }
  }

  uint32_t const isr_context = save_and_disable_interrupts();

  // RP2040 is 120 MHz, 120 cycle = 1us = 1000 ns
  // Neopixel is 800 KHz, 1T = 1.25 us = 150 nop
  while (1) {
    if (p & bitMask) {
      // T1H 0,8 us = 96 - 1 = 95 nop
      sio_hw->gpio_set = PIN_MASK;
      __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");

      // T1L 0,45 = 54 - 10 (ifelse) - 5 (overhead) = 44 nop
      sio_hw->gpio_clr = PIN_MASK;
      __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop;");
    } else {
      // T0H 0,4 us = 48 - 1 = 47 nop
      sio_hw->gpio_set = PIN_MASK;
      __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop;");

      // T0L 0.85 us = 102 - 10 (ifelse) - 5 (overhead) = 87 nop
      sio_hw->gpio_clr = PIN_MASK;
      __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
                     "nop; nop; nop; nop;");
    }

    if (bitMask >>= 1) {
      // not full byte, shift to next bit
      __asm volatile("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
    } else {
      // probably take 10 nops
      // if a full byte is sent, next to another byte
      if (ptr >= end) {
        break;
      }
      p = *ptr++;
      bitMask = 0x80;
    }
  }

  restore_interrupts(isr_context);

  end_us = micros();
}
