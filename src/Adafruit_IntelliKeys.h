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

#ifndef ADAFRUIT_INTELLIKEYS_H_
#define ADAFRUIT_INTELLIKEYS_H_

#include "Adafruit_TinyUSB.h"
#include "intellikeysdefs.h"

class Adafruit_IntelliKeys {
public:
  Adafruit_IntelliKeys(void);

  void begin(void);
  bool mount(uint8_t daddr);
  void umount(uint8_t daddr);

  // Internal
  void process_download_firmware(tuh_xfer_t *xfer);

private:
  uint8_t _daddr;
  uint8_t _state;

  bool start(void);
  bool postCommand(uint8_t *command);

  bool ezusb_StartDevice(void);
  bool ezusb_DownloadIntelHex(INTEL_HEX_RECORD const *record);
  bool ezusb_8051Reset(uint8_t resetBit);

  // internal helper
  bool ezusb_load_xfer(uint8_t brequest, uint16_t addr, const void *buffer,
                       uint16_t len);
  bool ezusb_downloadHex(INTEL_HEX_RECORD const *record, bool internal_ram);
};

#endif
