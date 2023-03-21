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
#else
#define IK_PRINTF(...)
#endif

//--------------------------------------------------------------------+
// Public API
//--------------------------------------------------------------------+

Adafruit_IntelliKeys::Adafruit_IntelliKeys(void) {
  // _daddr = 0;
  _state = 0;
}

void Adafruit_IntelliKeys::begin(void) {}

bool Adafruit_IntelliKeys::mount(uint8_t daddr) {
  uint16_t vid, pid;
  tuh_vid_pid_get(daddr, &vid, &pid);

  Serial.printf("VID = %04x, PID = %04x\r\n", vid, pid);

  if (vid != IK_VID)
    return false;

  // _daddr = daddr;

  if (pid == IK_PID_FWLOAD) {
    IK_PRINTF("IK mounted without firmware\n");
    ezusb_StartDevice(daddr);
  } else if (pid == IK_PID_RUNNING) {
    IK_PRINTF("IK mounted running firmware\n");
  } else {
    return false;
  }

  return true;
}

void Adafruit_IntelliKeys::umount(uint8_t daddr) {}

//--------------------------------------------------------------------+
// Private
//--------------------------------------------------------------------+

bool Adafruit_IntelliKeys::start(void) { return false; }

bool Adafruit_IntelliKeys::PostCommand(uint8_t *command) { return false; }

// Make an firmware load control request
bool Adafruit_IntelliKeys::ezusb_load_xfer(uint8_t daddr, uint8_t bRequest,
                                           uint16_t addr, const void *buffer,
                                           uint16_t len) {
  tusb_control_request_t const request = {
      .bmRequestType_bit = {.recipient = TUSB_REQ_RCPT_DEVICE,
                            .type = TUSB_REQ_TYPE_VENDOR,
                            .direction = TUSB_DIR_OUT},
      .bRequest = bRequest,
      .wValue = tu_htole16(addr),
      .wIndex = 0,
      .wLength = tu_htole16(len),
  };

  tuh_xfer_t xfer = {.daddr = daddr,
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

bool Adafruit_IntelliKeys::ezusb_StartDevice(uint8_t daddr) {
  IK_PRINTF("enter Ezusb_StartDevice\n");

  xfer_result_t result = XFER_RESULT_INVALID;
  tuh_interface_set(daddr, 0, 0, NULL, (uintptr_t)&result);

  if (result != XFER_RESULT_SUCCESS) {
    IK_PRINTF("Failed to Set Interface\n");
    return false;
  }

  // First download loader firmware.  The loader firmware implements a vendor
  // specific command that will allow us to anchor load to external ram
  ezusb_8051Reset(daddr, 1);
  ezusb_DownloadIntelHex(daddr, ik_loader);
  ezusb_8051Reset(daddr, 0);

  // Now download the device firmware
  ezusb_DownloadIntelHex(daddr, ik_firmware);
  ezusb_8051Reset(daddr, 1);
  ezusb_8051Reset(daddr, 0);

  IK_PRINTF("exit Ezusb_StartDevice\n");

  return true;
}

bool Adafruit_IntelliKeys::ezusb_DownloadIntelHex(
    uint8_t daddr, INTEL_HEX_RECORD const *hex_record) {
  // The download must be performed in two passes.  The first pass loads all of
  // the external addresses, and the 2nd pass loads to all of the internal
  // addresses. why?  because downloading to the internal addresses will
  // probably wipe out the firmware running on the device that knows how to
  // receive external ram downloads.

  // First download all the records that go in external ram
  ezusb_downloadHex(daddr, hex_record, false);

  // Now download all of the records that are in internal RAM.  Before starting
  // the download, stop the 8051.
  ezusb_8051Reset(daddr, 1);

  ezusb_downloadHex(daddr, hex_record, true);

  return false;
}

bool Adafruit_IntelliKeys::ezusb_downloadHex(uint8_t daddr,
                                             INTEL_HEX_RECORD const *ptr,
                                             bool internal_ram) {
  // First download all the records that go in external ram
  while (ptr->Type == 0) {
    if (INTERNAL_RAM(ptr->Address) == internal_ram) {
      // IK_PRINTF("Downloading %d bytes to 0x%x\n", ptr->Length, ptr->Address);

      uint8_t const bRequest =
          internal_ram ? ANCHOR_LOAD_INTERNAL : ANCHOR_LOAD_EXTERNAL;
      if (!ezusb_load_xfer(daddr, bRequest, ptr->Address, ptr->Data,
                           ptr->Length)) {
        IK_PRINTF("Failed to load hex file");
        return false;
      }
    }

    ptr++;
  }

  return true;
}

bool Adafruit_IntelliKeys::ezusb_8051Reset(uint8_t daddr, uint8_t resetBit) {
  return ezusb_load_xfer(daddr, ANCHOR_LOAD_INTERNAL, CPUCS_REG, &resetBit, 1);
}
