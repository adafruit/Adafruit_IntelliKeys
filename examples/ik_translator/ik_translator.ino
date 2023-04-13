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

// Pin D+ for host, D- = D+ + 1
#ifndef PIN_USB_HOST_DP
#define PIN_USB_HOST_DP 20
#endif

// Pin for enabling Host VBUS. comment out if not used
#ifndef PIN_5V_EN
#define PIN_5V_EN 22
#endif
#ifndef PIN_5V_EN_STATE
#define PIN_5V_EN_STATE 1
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

//--------------------------------------------------------------------+
// Setup and Loop on Core0
//--------------------------------------------------------------------+

void setup() {
  Serial.begin(115200);
  usb_keyboard.begin();
  usb_mouse.begin();

  // while ( !Serial ) delay(10);   // wait for native usb

  Serial.println("IntelliKeys USB Adapter");
}

bool checkNewKeyboardReport(hid_keyboard_report_t const *report,
                            ik_report_keyboard_t *ik_keyboard) {
  if (ik_keyboard->modifier != 0 &&
      !(report->modifier & ik_keyboard->modifier)) {
    return true;
  }

  for (uint8_t i = 0; i < 6; i++) {
    if (ik_keyboard->keycode == report->keycode[i]) {
      return false;
    }
  }

  return true;
}

void combineMouseReport(hid_mouse_report_t *report,
                        ik_report_mouse_t *ik_mouse) {
  report->buttons |= ik_mouse->buttons;
  report->x += ik_mouse->x;
  report->y += ik_mouse->y;
}

void scanMembraneAndSwitch(void) {
  static hid_keyboard_report_t kb_prev_report = {0};
  static bool kb_has_report = false;

  static uint8_t mouse_prev_buttons = 0;

  if (!IKeys.IsOpen()) {
    return;
  }

  IKOverlay *overlay;
  if (IKeys.HasStandardOverlay()) {
    overlay = IKeys.GetCurrentOverlay();
  } else {
    // not support non-std overlay yet
    return;
  }

  hid_keyboard_report_t kb_report = {0};
  uint8_t kb_count = 0;

  hid_mouse_report_t mouse_report = {0};

  const uint8_t(*mb)[IK_RESOLUTION_Y] = IKeys.getMembrane();

  // scan membrane
  for (uint8_t i = 0; i < IK_RESOLUTION_X; i++) {
    for (uint8_t j = 0; j < IK_RESOLUTION_Y; j++) {
      if (mb[i][j] == 1) {
        ik_report_t ik_report;
        overlay->getMembraneReport(i, j, &ik_report);

        if (ik_report.type == IK_REPORT_TYPE_KEYBOARD) {
          // Serial.printf(
          //    "rol = %u, col = %u, modifier = %02X, keycode = %02X\r\n", i, j,
          //    ik_report.keyboard.modifier, ik_report.keyboard.keycode);
          if (checkNewKeyboardReport(&kb_report, &ik_report.keyboard)) {
            kb_report.modifier |= ik_report.keyboard.modifier;
            kb_report.keycode[kb_count] = ik_report.keyboard.keycode;

            kb_count++;
            if (kb_count >= 6) {
              break;
            }
          }
        } else if (ik_report.type == IK_REPORT_TYPE_MOUSE) {
          //          Serial.printf(
          //              "rol = %u, col = %u, buttons = %02X, x = %d, y =
          //              %d\r\n", i, j, ik_report.mouse.buttons,
          //              ik_report.mouse.x, ik_report.mouse.y);
          combineMouseReport(&mouse_report, &ik_report.mouse);
        }
      }
    }
  }

  // TODO scan switch

#if 1
  if (kb_count) {
    // send only if kb_report is changed since last time
    if (memcmp(&kb_prev_report, &kb_report, sizeof(kb_report))) {
      usb_keyboard.sendReport(0, &kb_report, sizeof(kb_report));
    }
    kb_has_report = true;
  } else {
    if (kb_has_report) {
      // has kb_report before, send empty kb_report to release all keys
      hid_keyboard_report_t null_report = {0};
      usb_keyboard.sendReport(0, &null_report, sizeof(null_report));
    }
    kb_has_report = false;
  }

  kb_prev_report = kb_report;

  if (mouse_report.buttons != mouse_prev_buttons || mouse_report.x != 0 ||
      mouse_report.y != 0) {
    // x,y is only 0, 1, -1
    enum { MOUSE_SCALE = 1 };
    mouse_report.x *= MOUSE_SCALE;
    mouse_report.y *= MOUSE_SCALE;

    // TODO check for IK_REPORT_MOUSE_DOUBLE_CLICK and
    // IK_REPORT_MOUSE_CLICK_HOLD
    usb_mouse.sendReport(0, &mouse_report, sizeof(mouse_report));
  }
  mouse_prev_buttons = mouse_report.buttons;
#endif
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

#if 0
// Invoked when device with hid interface is mounted
// Report descriptor is also available for use.
// tuh_hid_parse_report_descriptor() can be used to parse common/simple enough
// descriptor. Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE,
// it will be skipped therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  (void)desc_report;
  (void)desc_len;
  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  Serial.printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  Serial.printf("VID = %04x, PID = %04x\r\n", vid, pid);

  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);
  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
    Serial.printf("HID Keyboard\r\n", vid, pid);
    if (!tuh_hid_receive_report(dev_addr, instance)) {
      Serial.printf("Error: cannot request to receive report\r\n");
    }
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  Serial.printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}

void remap_key(hid_keyboard_report_t const* original_report, hid_keyboard_report_t* remapped_report)
{
  memcpy(remapped_report, original_report, sizeof(hid_keyboard_report_t));

  // only remap if not empty report i.e key released
  for(uint8_t i=0; i<6; i++) {
    if (remapped_report->keycode[i] != 0) {
      // Note: we ignore right shift here
      remapped_report->modifier ^= KEYBOARD_MODIFIER_LEFTSHIFT;
      break;
    }
  }
}
#endif

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const *report, uint16_t len) {
  IKeys.hid_reprot_received_cb(dev_addr, instance, report, len);
}

} // extern C
