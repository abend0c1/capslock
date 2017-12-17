/*
  CAP! CapsLock Light
  Copyright (C) 2016-2017 Andrew J. Armstrong

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

/*
-------------------------------------------------------------------------------

NAME     - CAP! CapsLock Light

FUNCTION - This is a USB capslock-light-on-a-stick that lights up when the
           CapsLock LED would normally be lit. Some Lenovo laptops do not have
           a CapsLock LED.
           
           By default, the SCROLL LOCK keystroke is sent to the host every
           60 seconds. This behaviour can be toggled by pressing the button
           on the CapsLock dongle. The SCROLL LOCK light, present on many
           keyboards, will flash to indicate the keystroke injection is
           active.


           By default, the SCROLL LOCK keystroke
           is sent to the host every 60 seconds to prevent the host from
           detecting a "time out" situation.

           The SCROLL LOCK light, present on many older keyboards, will flash
           to indicate the keystroke injection is active. The LED on this
           device will also briefly blink for each keystroke injection.


           By default, the SCROLL LOCK keystroke
           is sent to the host every 60 seconds to prevent the host from
           detecting a "time out" situation.

           The SCROLL LOCK light, present on many older keyboards, will flash
           to indicate the keystroke injection is active. The LED on this
           device will also briefly blink for each keystroke injection.


FEATURES - 1. Absolutely NO HOST DRIVERS required.
           2. The user can press the button to toggle the sending of "keep
              alive" keystrokes.

PIN USAGE -                     PIC16F1455
                           .------------------.
            Vdd +5V    --- | RE3  1    14 RB7 | --- Vss 0V -------.
                   n/c --- | RA5  2    13 RA0 | <-- USB D+        |
            LED        <-- | RA4  3    12 RA1 | <-- USB D-        |
           ~MCLR       --> | RA3  4    11 RB4 | --- Vusb -----||--' 2 x 100 nF
                   n/c --- | RC5  5    10 RC0 | <-- PGD
                   n/c --- | RC4  6     9 RC1 | <-- PGC
                   n/c --- | RC3  7     8 RC2 | --- n/c
                           '------------------'


CONFIG    - The PIC16F1455 configuration fuses should be set as follows (items
            marked with '*' are essential for this program to operate correctly):

            CONFIG1  3EA4 0011111010100100
                          xx               =  Unimplemented
                            x              =  FCMEM:     1=Fail-Safe Clock Monitor is enabled
                             x             =  IESO:      1=Internal/External Switchover mode is enabled
                              x            =  ~CLKOUTEN: 1=CLKOUT function is disabled. I/O or oscillator function on CLKOUT pin
                               xx          =  BOREN:     11=BOR enabled
                                 x         =  Unimplemented
                                  x        =  ~CP:       1=Program memory code protection is disabled
                                   x       = *MCLRE:     0=MCLR internally disabled
                                    x      =  ~PWRTE:    1=Power-Up Timer disabled
                                     xx    =  WDTE:      00=Watchdog Timer disabled
                                       xxx = *FOSC:      100=INTOSC oscillator: I/O function on OSC1 pin

            CONFIG2  1EC3 0001111011000011
                          xx               =  Unimplemented
                            x              =  LVP:       0=Low Voltage Programming: High-voltage on ~MCLR must be used for programming
                             x             =  ~DEBUG:    1=In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins
                              x            =  ~LPBOR:    1=Low-Power Brown-out Reset is disabled
                               x           =  BORV:      1=Brown-out Reset voltage (Vbor), low trip point selected
                                x          =  STVREN:    1=Stack Overflow or Underflow will cause a Reset
                                 x         = *PLLEN:     0=PLL is disabled (it will be enabled under software control)
                                  x        = *PLLMULT:   1=3x PLL Output Frequency is selected
                                   x       = *USBLSCLK:  1=USB Clock divide-by 8 (48 MHz system input clock expected)
                                    xx     = *CPUDIV:    00=No CPU system clock divide
                                      xx   =  Unimplemented
                                        xx =  WRT:       11=Flash Memory Self-Write Protection bits: Write protection off



OPERATION - Plug it in (relaxen und watschen der blinkenlichten)

NOTES    - 1. The source code is written in MikroC Pro from mikroe.com


REFERENCE - USB Human Interface Device Usage Tables at:
            http://www.usb.org/developers/devclass_docs/Hut1_12v2.pdf

            USB Device Class Definition for Human Interface Devices (HID) at:
            http://www.usb.org/developers/devclass_docs/HID1_11.pdf

            USB 2.0 Specification at:
            http://www.usb.org/developers/docs/usb_20_101111.zip
            Specifically, the file in that zip archive called usb_20.pdf

AUTHORS  - Init Name                 Email
           ---- -------------------- ------------------------------------------
           AJA  Andrew J. Armstrong  androidarmstrong@gmail.com

HISTORY  - Date     Ver   By  Reason (most recent at the top please)
           -------- ----- --- -------------------------------------------------
           20171219 2.01  AJA Converted to a BOOT keyboard and removed mouse
           20171102 2.00  AJA Removed button from the PCB
           20170413 1.01  AJA Added micro mouse movements too
           20161030 1.00  AJA Initial version

-------------------------------------------------------------------------------
*/
#include <stdint.h>
#include <built_in.h>
#include "USBdsc.h"
#include "capslock.h"


void enableUSB()
{
  uint8_t i;

  usbToHost[0] = 0;                      // No modifiers
  usbToHost[1] = 0;                      // Reserved for OEM
  usbToHost[2] = 0;                      // No key pressed
  usbToHost[3] = 0;                      // No key pressed
  usbToHost[4] = 0;                      // No key pressed
  usbToHost[5] = 0;                      // No key pressed
  usbToHost[6] = 0;                      // No key pressed
  usbToHost[7] = 0;                      // No key pressed
  bUSBReady = FALSE;

  while (!bUSBReady)
  {
    HID_Enable(&usbFromHost, &usbToHost);
    for (i=0; !bUSBReady && i < 50; i++) // Keep trying for up to 50 x 100 ms = 5 seconds
    {
      Delay_ms(100);
      CAPSLOCK_LED = ON;
      bUSBReady = HID_Write(&usbToHost, sizeof usbToHost) != 0; // Copy to USB buffer and try to send
      CAPSLOCK_LED = OFF;
    }
    if (!bUSBReady)
    {
      HID_Disable();
      Delay_ms(5000);
    }
  }
  Delay_ms(250);
}

void disableUSB()
{
  HID_Disable();
  bUSBReady = FALSE;
}



void Prolog()
{
  ANSELA = 0b00000000;    // Configure all PORTA bits as digital
  ANSELC = 0b00000000;    // Configure all PORTC bits as digital

  LATA = 0;
  LATC = 0;

  //        76543210
  TRISA = 0b00000000;     // 1=Input, 0=Output
  TRISC = 0b00000000;

  NOT_WPUEN_bit = 0;      // Enable weak pull-ups <-- Expect pain if this is not enabled

// You can enable the PLL and set the PLL multipler two ways:
// 1. At compile time - setting configuration fuses: CONFIG2.PLLEN and CONFIG2.PLLMULT
//    ...but only if you use an External Clock (EC).
// 2. At run time - setting OSCCON.SPLLEN and OSCCON.SPLLMULT
//    ...but only if you use a High Speed crystal (HS), or
//       the High Frequency Internal Oscillator (HFINTOSC).
// This project uses the HFINTOSC set to run at 16 MHz and the only valid
// PLL multiplier for USB is x3.
  OSCCON = 0b11111100;
//           x              = SPPLEN:   1=PLL is enabled
//            x             = SPLLMULT: 1=3x PLL is enabled
//             xxxx         = IRCF:     1111=16 MHz or 48 MHz
//                 xx       = SCS:      00=Clock determined by FOSC in Configuration Words

  while (!HFIOFS_bit);    // Wait for HFINTOSC to stabilise
  while(!PLLRDY_bit);     // Wait for PLL to lock

  ACTEN_bit = 0;          // Disable Active Clock Tuning, then...
  ACTSRC_bit = 1;         // Tune HFINTOSC to match USB host clock
  ACTEN_bit = 1;          // Enable Active Clock Tuning
//  while (!ACTLOCK_bit);   // Wait until HFINTOSC is successfully tuned

  cFlags = 0;             // Reset all flags
  bKeepAlive = 1;         // Enable "keep alive" keystroke injection

  // Set up USB (only do this while the USB module is disabled)
  UPUEN_bit = 1;          // USB On-chip pull-up enable
  FSEN_bit = 1;           // 1 = USB Full Speed enabled (requires 48 MHz USB clock)
                          // 0 = USB Full Speed disabled (requires 6 MHz USB clock)

// Timer1 is used for human-scale delays (Timer1 is not always enabled)
  T1CON   = 0b00110010;
//            xx             TMR1CS:  00=Timer1 clock source is instruction clock (Fosc/4)
//              xx           T1CKPS:  11=Timer1 prescale value is 1:8
//                x          T1OSCEN: 0=Low Power oscillator circuit disabled
//                 x        ~T1SYNC:  0=Synchronize asynchronous clock input with system clock (Fosc)
//                  x        Unimplemented
//                   x       TMR1ON:  0=Timer1 is off
// Timer1 tick rate = 48 MHz FOSC/4/8 = 1.5 MHz (667 ns)
// Timer1 interrupt rate = 1.5 MHz / 65536 = 22.9 times per second

  CAPSLOCK_LED = OFF;

//----------------------------------------------------------------------------
// Let the interrupts begin
//----------------------------------------------------------------------------

  TMR1IE_bit = 1;         // Enable Timer1 interrupts (for keepalive)
  PEIE_bit = 1;           // Enable peripheral interrupts
  GIE_bit = 1;            // Enable global interrupts

  enableUSB();            // Enable USB interface
}


void pressKey (uint8_t key)
{
  usbToHost[0] = 0x00;                   // Key modifier usages
  usbToHost[1] = 0x00;                   // Reserved
  usbToHost[2] = key;                    // Key usage code
  while (!bUSBReady = HID_Write(&usbToHost, sizeof usbToHost)); // Send a "SCROLL LOCK key pressed" message to the host
  usbToHost[2] = 0b00000000;             // No key pressed
  while (!bUSBReady = HID_Write(&usbToHost, sizeof usbToHost)); // Send a "no key pressed" message to the host
}


#define KEEP_ALIVE_INTERVAL 60

void main()
{
  Prolog();
  nRemainingTimerTicks = INTERVAL_IN_SECONDS(KEEP_ALIVE_INTERVAL);
  TMR1ON_bit = bKeepAlive;   // Enable keepalive interrupts
  while (1)
  {
    if (BUTTON_PRESSED)
    {
      Delay_ms(25);                   // Cheap button debounce
      if (BUTTON_PRESSED)             // If button still pressed after debounce delay
      {
        TMR3ON_bit = 0;               // Prevent keystroke injection while the button is pressed
        CAPSLOCK_LED ^= 1;            // Flash LED to acknowledge button press
        while (BUTTON_PRESSED);       // Wait until button is released
        CAPSLOCK_LED ^= 1;            // Restore LED status
        bKeepAlive ^= 1;              // Toggle injecting "keep alive" keystrokes
        nRemainingTimerTicks = INTERVAL_IN_SECONDS(KEEP_ALIVE_INTERVAL);
        TMR3ON_bit = bKeepAlive;
      }
    }
    if (bUSBReady)
    {
      if (HID_Read() == 1)   // If a (complete) host LED indication response is available
      {
        leds.byte = usbFromHost[0];   // Remember the most recent LED status change
        CAPSLOCK_LED = leds.bits.CapsLock; // Make the CAPSLOCK light match the CAPSLOCK state
        nRemainingTimerTicks = INTERVAL_IN_SECONDS(KEEP_ALIVE_INTERVAL); // User has pressed CAPSLOCK, so check again in a little while
      }

      if (nRemainingTimerTicks == 0)  // If the keepalive timer has popped
      {
        CAPSLOCK_LED ^= 1;            // Flash LED
        pressKey(SCROLL_LOCK_KEY);    // Press harmless key (turns on Scroll Lock light on old keyboards)
        pressKey(SCROLL_LOCK_KEY);    // Press harmless key (to reset Scroll Lock to its original state)
        CAPSLOCK_LED ^= 1;            // Flash LED
        nRemainingTimerTicks = INTERVAL_IN_SECONDS(KEEP_ALIVE_INTERVAL);  // Check again in a little while
      }
    }
    else
    {
      disableUSB();
      enableUSB();
    }
  }
}

void interrupt()               // High priority interrupt service routine
{
  if (USBIF)
  {
    USB_Interrupt_Proc();      // Always give the USB module first opportunity to process (resets USBIF)
  }
  if (TMR1IF_bit)              // Timer1 interrupt? (22.9 times/second)
  {
    nRemainingTimerTicks--;    // Decrement delay count
    TMR1IF_bit = 0;            // Clear the Timer1 interrupt flag
  }
}