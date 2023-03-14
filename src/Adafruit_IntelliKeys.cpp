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

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

#define IK_PRINTF(...)
// #define IK_PRINTF(...) Serial.printf(__VA_ARGS__)

extern const INTEL_HEX_RECORD ik_loader[];
extern const INTEL_HEX_RECORD ik_firmware[];

//------------- IMPLEMENTATION -------------//
Adafruit_IntelliKeys::Adafruit_IntelliKeys(void)
{

}

bool Adafruit_IntelliKeys::start(void)
{
  return false;
}

bool Adafruit_IntelliKeys::PostCommand(uint8_t *command)
{
  return false;
}

bool Adafruit_IntelliKeys::ezusb_StartDevice(void)
{
  IK_PRINTF("enter Ezusb_StartDevice\n");

  // First download loader firmware.  The loader firmware implements a vendor
  // specific command that will allow us to anchor load to external ram
  ezusb_8051Reset(1);
  ezusb_DownloadIntelHex(ik_loader);
  ezusb_8051Reset(0);

  // Now download the device firmware
  ezusb_DownloadIntelHex(ik_firmware);
  ezusb_8051Reset(1);
  ezusb_8051Reset(0);

  IK_PRINTF("exit Ezusb_StartDevice\n");

  return true;
}

bool Adafruit_IntelliKeys::ezusb_DownloadIntelHex(INTEL_HEX_RECORD const* hex_record)
{
  // The download must be performed in two passes.  The first pass loads all of the
  // external addresses, and the 2nd pass loads to all of the internal addresses.
  // why?  because downloading to the internal addresses will probably wipe out the firmware
  // running on the device that knows how to receive external ram downloads.

  // First download all the records that go in external ram
  _downloadHex(hex_record, false);

  // Now download all of the records that are in internal RAM.  Before starting
  // the download, stop the 8051.
  ezusb_8051Reset(1);

  _downloadHex(hex_record, true);

  return false;
}

bool Adafruit_IntelliKeys::_downloadHex(INTEL_HEX_RECORD const* ptr, bool internal_ram)
{
  // First download all the records that go in external ram
  while(ptr->Type == 0)
  {
    if (INTERNAL_RAM(ptr->Address) == internal_ram)
    {
//      urb->UrbControlVendorClassRequest.TransferBufferLength = ptr->Length;
//      urb->UrbControlVendorClassRequest.TransferBuffer = ptr->Data;
//      urb->UrbControlVendorClassRequest.Request = (internal) ? ANCHOR_LOAD_INTERNAL : ANCHOR_LOAD_EXTERNAL;
//      urb->UrbControlVendorClassRequest.Value = ptr->Address;
//      urb->UrbControlVendorClassRequest.Index = 0;

//        uint32_t rv = pUsb->ctrlReq(bAddress, 0, 0x40,
//                (internal)?ANCHOR_LOAD_INTERNAL:ANCHOR_LOAD_EXTERNAL,
//                (uint8_t)pHex->Address, (uint8_t)(pHex->Address>>8),
//                0, pHex->Length, pHex->Length, pHexBuf, NULL);

      IK_PRINTF("Downloading %d bytes to 0x%x\n", ptr->Length, ptr->Address);
    }
  }

  return true;
}


bool Adafruit_IntelliKeys::ezusb_8051Reset(uint8_t resetBit)
{
//      urb->UrbControlVendorClassRequest.TransferBufferLength = 1;
//      urb->UrbControlVendorClassRequest.TransferBuffer = &resetBit;
//      urb->UrbControlVendorClassRequest.TransferBufferMDL = NULL;
//      urb->UrbControlVendorClassRequest.Request = ANCHOR_LOAD_INTERNAL;
//      urb->UrbControlVendorClassRequest.Value = CPUCS_REG;
//      urb->UrbControlVendorClassRequest.Index = 0;

//    reg_value = resetBit;
//    uint32_t rv = pUsb->ctrlReq(bAddress, 0, 0x40, ANCHOR_LOAD_INTERNAL,
//            (uint8_t)CPUCS_REG, (uint8_t)(CPUCS_REG>>8), 0, 1, 1, &reg_value, NULL);

  return false;
}
