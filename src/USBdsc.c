/*
  CAP! CapsLock Light
  Copyright (C) 2017 Andrew J. Armstrong

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307  USA

  Author:
  Andrew J. Armstrong <androidarmstrong@gmail.com>

*/
#include <stdint.h>
#include "USBdsc.h"

#define STRING_INDEX_MANUFACTURER  1
#define STRING_INDEX_PRODUCT       2
#define STRING_INDEX_SERIAL        0
#define STRING_INDEX_CONFIG        0

#define IBM       0x04B3
#define IBM_SK_8845B        0x3019
#define IBM_SK_8815         0x301B


const uint16_t USB_VENDOR_ID  = IBM;
const uint16_t USB_PRODUCT_ID = IBM_SK_8845B;
const uint16_t USB_PRODUCT_VERSION = 0x0122; // Increment this (vv.rr) each time your device's firmware changes significantly
const uint8_t  USB_SELF_POWER = 0x80;        // 0x80 = Bus powered, 0xC0 = Self powered
const uint8_t  USB_MAX_POWER  = 50;          // Bus power required in units of 2 mA
const uint8_t  USB_TRANSFER_TYPE = 0x03;     // 0x03 = Interrupt transfers
const uint8_t  EP_IN_INTERVAL = 5;           // Measured in frame counts i.e. 1 ms units for Low Speed (1.5 Mbps) or Full Speed (12 Mbps), and 125 us units for High Speed (480 Mbps)
                                             // This device supports either Low Speed (FSEN=0, 6 MHz USB clock) or Full Speed (FSEN=1, 48 MHz USB clock) mode.
                                             // n x 1 millisecond units (for USB Low/Full Speed devices)
                                             // 2**(n-1) x 125 microsecond units (for USB2 High Speed devices)
                                             // The Host will interrupt the PIC for keyboard input this often.

const uint8_t  EP_OUT_INTERVAL = 5;          // n x 1 millisecond units (for USB Low/Full Speed devices)
                                             // 2**(n-1) x 125 microsecond units (for USB2 High Speed devices)
                                             // The Host will interrupt the PIC for LED status output at most this often (if LED status change is pending).

const uint8_t  USB_INTERRUPT = 1;
const uint8_t  USB_HID_EP = 1;               // End Point number

/* Device Descriptor */
const struct
{
    uint8_t  bLength;               // bLength         - Descriptor size in bytes (12h)
    uint8_t  bDescriptorType;       // bDescriptorType - The constant DEVICE (01h)
    uint16_t bcdUSB;                // bcdUSB          - USB specification release number (BCD)
    uint8_t  bDeviceClass;          // bDeviceClass    - Class Code
    uint8_t  bDeviceSubClass;       // bDeviceSubClass - Subclass code
    uint8_t  bDeviceProtocol;       // bDeviceProtocol - Protocol code
    uint8_t  bMaxPacketSize0;       // bMaxPacketSize0 - Maximum packet size for endpoint 0
    uint16_t idVendor;              // idVendor        - Vendor ID
    uint16_t idProduct;             // idProduct       - Product ID
    uint16_t bcdDevice;             // bcdDevice       - Device release number (BCD)
    uint8_t  iManufacturer;         // iManufacturer   - Index of string descriptor for the manufacturer (0 means none)
    uint8_t  iProduct;              // iProduct        - Index of string descriptor for the product (0 means none)
    uint8_t  iSerialNumber;         // iSerialNumber   - Index of string descriptor for the serial number (0 means none)
    uint8_t  bNumConfigurations;    // bNumConfigurations - Number of possible configurations
} device_dsc = 
  {
      18,                           // bLength
      0x01,                         // bDescriptorType
      0x0110,                       // bcdUSB   (USB 1.1)
      0x00,                         // bDeviceClass
      0x00,                         // bDeviceSubClass
      0x00,                         // bDeviceProtocol
      8,                            // bMaxPacketSize0
      USB_VENDOR_ID,                // idVendor
      USB_PRODUCT_ID,               // idProduct
      USB_PRODUCT_VERSION,          // bcdDevice
      STRING_INDEX_MANUFACTURER,    // iManufacturer
      STRING_INDEX_PRODUCT,         // iProduct
      STRING_INDEX_SERIAL,          // iSerialNumber
      0x01                          // bNumConfigurations
  };

/*
  Keyboard Input Report (PIC --> Host) 4 bytes as follows:
    .---------------------------------------.
    |          REPORT_ID_KEYBOARD           | IN: Report Id
    |---------------------------------------|
    |RGUI|RALT|RSHF|RCTL|LGUI|LALT|LSHF|LCTL| IN: Ctrl/Shift/Alt/GUI keys on left and right hand side of keyboard
    |---------------------------------------|
    |                (pad)                  | IN: pad (strangely, this pad byte is necessary)
    |---------------------------------------|
    |                 Key                   | IN: Key that is currently pressed
    '---------------------------------------'

  Output Report (PIC <-- Host) 2 bytes as follows:

    .---------------------------------------.
    |          REPORT_ID_KEYBOARD           | OUT: Report Id
    |---------------------------------------|
    |    |    |    |    |    |SCRL|CAPL|NUML| OUT: NumLock,CapsLock,ScrollLock - and 5 unused pad bits
    '---------------------------------------'

  Mouse Input Report (PIC --> Host) 2 bytes as follows:
    .---------------------------------------.
    |          REPORT_ID_MOUSE              | IN: Report Id
    |---------------------------------------|
    |                 X                     | IN: Relative movement along the X-axis
    '---------------------------------------'
*/


#undef STRING
#define STRING \
  0x05, 0x01,                  /* (GLOBAL) USAGE_PAGE         0x0001 Generic Desktop Page */ \
  0x09, 0x06,                  /* (LOCAL)  USAGE              0x00010006 Keyboard (CA=Application Collection) */ \
  0xA1, 0x01,                  /* (MAIN)   COLLECTION         0x01 Application (Usage=0x00010006: Page=Generic Desktop Page, Usage=Keyboard, Type=CA) */ \
  0x85, REPORT_ID_KEYBOARD,    /*   (GLOBAL) REPORT_ID          0x4B (75) 'K' */ \
  0x05, 0x07,                  /*   (GLOBAL) USAGE_PAGE         0x0007 Keyboard/Keypad Page */ \
  0x19, 0xE0,                  /*   (LOCAL)  USAGE_MINIMUM      0x000700E0 Keyboard Left Control (DV=Dynamic Value) */ \
  0x29, 0xE7,                  /*   (LOCAL)  USAGE_MAXIMUM      0x000700E7 Keyboard Right GUI (DV=Dynamic Value) */ \
  0x25, 0x01,                  /*   (GLOBAL) LOGICAL_MAXIMUM    0x01 (1) */ \
  0x75, 0x01,                  /*   (GLOBAL) REPORT_SIZE        0x01 (1) Number of bits per field */ \
  0x95, 0x08,                  /*   (GLOBAL) REPORT_COUNT       0x08 (8) Number of fields */ \
  0x81, 0x02,                  /*   (MAIN)   INPUT              0x00000002 (8 fields x 1 bit) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap */ \
  0x75, 0x08,                  /*   (GLOBAL) REPORT_SIZE        0x08 (8) Number of bits per field */ \
  0x95, 0x01,                  /*   (GLOBAL) REPORT_COUNT       0x01 (1) Number of fields */ \
  0x81, 0x03,                  /*   (MAIN)   INPUT              0x00000003 (1 field x 8 bits) 1=Constant 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap */ \
  0x26, 0xFF, 0x00,            /*   (GLOBAL) LOGICAL_MAXIMUM    0x00FF (255) */ \
  0x19, 0x00,                  /*   (LOCAL)  USAGE_MINIMUM      0x00070000 Keyboard No event indicated (Sel=Selector) <-- Redundant: USAGE_MINIMUM is already 0x0000 */ \
  0x2A, 0xFF, 0x00,            /*   (LOCAL)  USAGE_MAXIMUM      0x000700FF */ \
  0x81, 0x00,                  /*   (MAIN)   INPUT              0x00000000 (1 field x 8 bits) 0=Data 0=Array 0=Absolute 0=Ignored 0=Ignored 0=PrefState 0=NoNull */ \
\
  0x75, 0x01,                  /*   (GLOBAL) REPORT_SIZE        0x01 (1) Number of bits per field */ \
  0x95, 0x03,                  /*   (GLOBAL) REPORT_COUNT       0x03 (3) Number of fields */ \
  0x05, 0x08,                  /*   (GLOBAL) USAGE_PAGE         0x0008 LED Indicator Page */ \
  0x19, 0x01,                  /*   (LOCAL)  USAGE_MINIMUM      0x00080001 Num Lock (OOC=On/Off Control) */ \
  0x29, 0x03,                  /*   (LOCAL)  USAGE_MAXIMUM      0x00080003 Scroll Lock (OOC=On/Off Control) */ \
  0x25, 0x01,                  /*   (GLOBAL) LOGICAL_MAXIMUM    0x01 (1) */ \
  0x91, 0x02,                  /*   (MAIN)   OUTPUT             0x00000002 (3 fields x 1 bit) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap */ \
  0x75, 0x05,                  /*   (GLOBAL) REPORT_SIZE        0x05 (5) Number of bits per field */ \
  0x95, 0x01,                  /*   (GLOBAL) REPORT_COUNT       0x01 (1) Number of fields */ \
  0x91, 0x03,                  /*   (MAIN)   OUTPUT             0x00000003 (1 field x 5 bits) 1=Constant 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap */ \
  0xC0,                        /* (MAIN)   END_COLLECTION     Application */ \
\
  0x05, 0x01,                  /* (GLOBAL) USAGE_PAGE         0x0001 Generic Desktop Page */ \
  0x09, 0x02,                  /* (LOCAL)  USAGE              0x00010002 Mouse (CA=Application Collection) */ \
  0xA1, 0x01,                  /* (MAIN)   COLLECTION         0x01 Application (Usage=0x00010002: Page=Generic Desktop Page, Usage=Mouse, Type=CA) */ \
  0x09, 0x01,                  /*   (LOCAL)  USAGE              0x00010001 Pointer (CP=Physical Collection) */ \
  0xA1, 0x00,                  /*   (MAIN)   COLLECTION         0x00 Physical (Usage=0x00010001: Page=Generic Desktop Page, Usage=Pointer, Type=CP) */ \
  0x85, REPORT_ID_MOUSE,       /*     (GLOBAL) REPORT_ID          0x4D (77) 'M' */ \
  0x09, 0x30,                  /*     (LOCAL)  USAGE              0x00010030 X (DV=Dynamic Value) */ \
  0x15, 0x81,                  /*     (GLOBAL) LOGICAL_MINIMUM    0x81 (-127) */ \
  0x25, 0x7F,                  /*     (GLOBAL) LOGICAL_MAXIMUM    0x7F (127) */ \
  0x75, 0x08,                  /*     (GLOBAL) REPORT_SIZE        0x08 (8) Number of bits per field */ \
  0x95, 0x01,                  /*     (GLOBAL) REPORT_COUNT       0x01 (1) Number of fields */ \
  0x81, 0x06,                  /*     (MAIN)   INPUT              0x00000006 (1 field x 8 bits) 0=Data 1=Variable 1=Relative 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap */ \
  0xC0,                        /*   (MAIN)   END_COLLECTION     Physical */ \
  0xC0,                        /* (MAIN)   END_COLLECTION     Application */

REPORT_DESCRIPTOR(hid_rpt_desc);  // Build the HID report descriptor structure (it MUST be called hid_rpt_desc)
const uint8_t  USB_HID_RPT_SIZE = sizeof hid_rpt_desc; // This constant MUST be defined

/* Configuration 1 Descriptor */
const uint8_t configDescriptor1[] =
{
    // Configuration Descriptor
    0x09,                   // bLength             - Descriptor size in bytes
    0x02,                   // bDescriptorType     - The constant CONFIGURATION (02h)
    WORD(41),               // wTotalLength        - The number of bytes in the configuration descriptor and all of its subordinate descriptors
    1,                      // bNumInterfaces      - Number of interfaces in the configuration
    1,                      // bConfigurationValue - Identifier for Set Configuration and Get Configuration requests
    STRING_INDEX_CONFIG,    // iConfiguration      - Index of string descriptor for the configuration (0 means no descriptor)
    USB_SELF_POWER,         // bmAttributes        - Self/bus power and remote wakeup settings
    USB_MAX_POWER,          // bMaxPower           - Bus power required in units of 2 mA

    // Interface Descriptor
    0x09,                   // bLength - Descriptor size in bytes (09h)
    0x04,                   // bDescriptorType - The constant Interface (04h)
    0,                      // bInterfaceNumber - Number identifying this interface
    0,                      // bAlternateSetting - A number that identifies a descriptor with alternate settings for this bInterfaceNumber.
    2,                      // bNumEndpoint - Number of endpoints supported not counting endpoint zero
    0x03,                   // bInterfaceClass - Class code  (0x03 = HID)
    0,                      // bInterfaceSubclass - Subclass code (0x00 = No Subclass)
    0,                      // bInterfaceProtocol - Protocol code (0x00 = No protocol)
                            // Valid combinations of Class, Subclass, Protocol are as follows:
                            // Class Subclass Protocol Meaning
                            //   3       0       0     Class=HID with no specific Subclass or Protocol:
                            //                         Can have ANY size reports (not just 8-byte reports)
                            //   3       1       1     Class=HID, Subclass=BOOT device, Protocol=keyboard:
                            //                         REQUIRES 8-byte reports in order for it to be recognised by BIOS when booting.
                            //                         That is because the entire USB protocol cannot be implemented in BIOS, so
                            //                         motherboard manufacturers have agreed to use a fixed 8-byte report during booting.
                            //   3       1       2     Class=HID, Subclass=BOOT device, Protocol=mouse
                            // The above information is documented in Appendix E.3 "Interface Descriptor (Keyboard)"
                            // of the "Device Class Definition for Human Interface Devices (HID) v1.11" document (HID1_11.pdf) from www.usb.org
    STRING_INDEX_PRODUCT,   // iInterface - Interface string index

    // HID Class-Specific Descriptor
    0x09,                   // bLength - Descriptor size in bytes.
    0x21,                   // bDescriptorType - This descriptor's type: 21h to indicate the HID class.
    WORD(0x0101),           // bcdHID - HID specification release number (BCD)
    0x00,                   // bCountryCode - Numeric expression identifying the country for localized hardware (BCD) or 00h.
    1,                      // bNumDescriptors - Number of subordinate report and physical descriptors.
    0x22,                   // bDescriptorType - The type of a class-specific descriptor that follows
    WORD(USB_HID_RPT_SIZE), // wDescriptorLength - Total length of the descriptor identified above

    // Endpoint Descriptor - Inbound to host (i.e. key press codes)
    0x07,                   // bLength - Descriptor size in bytes (07h)
    0x05,                   // bDescriptorType - The constant Endpoint (05h)
    USB_HID_EP | 0x80,      // bEndpointAddress - Endpoint number (0x01) and direction (0x80 = IN to host)
    USB_TRANSFER_TYPE,      // bmAttributes - Transfer type and supplementary information
    WORD(8),                // wMaxPacketSize - Maximum packet size supported
                            // This determines the size of the transmission time slot allocated to this device
    EP_IN_INTERVAL,         // bInterval - Service interval or NAK rate

    // Endpoint Descriptor - Outbound from host (i.e. LED indicator status bits)
    0x07,                   // bLength - Descriptor size in bytes (07h)
    0x05,                   // bDescriptorType - The constant Endpoint (05h)
    USB_HID_EP,             // bEndpointAddress - Endpoint number (0x01) and direction (0x00 = OUT from host)
    USB_TRANSFER_TYPE,      // bmAttributes - Transfer type and supplementary information
    WORD(8),                // wMaxPacketSize - Maximum packet size supported
                            // This determines the size of the transmission time slot allocated to this device
    EP_OUT_INTERVAL         // bInterval - Service interval or NAK rate
};



#undef STRING
#define STRING 0x0409
STRING_DESCRIPTOR(Language);      // Build the language code string descriptor

#undef STRING
#define STRING 'L','i','t','e','-','O','n',' ','T','e','c','h','n','o','l','o','g','y',' ','C','o','r','p','.'
STRING_DESCRIPTOR(Manufacturer);  // Build the manufacturer string descriptor

#undef STRING
#define STRING 'T','h','i','n','k','P','a','d',' ','U','S','B',' ','K','e','y','b','o','a','r','d',' ','w','i','t','h',' ','T','r','a','c','k','P','o','i','n','t'
STRING_DESCRIPTOR(Product);       // Build the product string descriptor

const uint8_t * USB_config_dsc_ptr[1];  // Array of configuration descriptors
const uint8_t * USB_string_dsc_ptr[3];  // Array of string descriptors

void USB_Init_Desc()
{
  USB_config_dsc_ptr[0] = &configDescriptor1;
  USB_string_dsc_ptr[0]                         = (const uint8_t *) &Language;    // Index 0 MUST be the language code array pointer
  USB_string_dsc_ptr[STRING_INDEX_MANUFACTURER] = (const uint8_t *) &Manufacturer;
  USB_string_dsc_ptr[STRING_INDEX_PRODUCT]      = (const uint8_t *) &Product;
}