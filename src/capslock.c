/*
  CAP! CapsLock Light
  Copyright (C) 2016 Andrew J. Armstrong

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

FUNCTION - This is a USB caps-lock-light-on-a-stick that shines when the 
           CapsLock LED would normally be lit. Some Lenovo laptops do not have
           a CapsLock LED.

FEATURES - 1. Absolutely NO HOST DRIVERS required.

PIN USAGE -                     PIC18F25K50
                           .------------------.
            MCLR       --> | RE3  1    28 RB7 | <-- PGD
            LED        <-- | RA0  2    27 RB6 | <-- PGC
                       <-- | RA1  3    26 RB5 | <--
                       <-- | RA2  4    25 RB4 | <--
                       <-- | RA3  5    24 RB3 | <--
                       <-- | RA4  6    23 RB2 | <--
                       <-- | RA5  7    22 RB1 | <--
            Ground     --- | VSS  8    21 RB0 | <--
            n/c        --- | RA7  9    20 VDD | --- +5V
                       <-- | RA6  10   19 VSS | --- Ground
                       <-- | RC0  11   18 RC7 | --> RX
                       <-- | RC1  12   17 RC6 | --> TX
                       <-- | RC2  13   16 RC5 | <-> USB D+
            2 x 100 nF --- | VUSB 14   15 RC4 | <-> USB D-
                           '------------------'


CONFIG    - The PIC18F25K50 configuration fuses should be set as follows (items
            marked with '*' are essential for this program to operate correctly):

            CONFIG1L 2B 00101011
                          1      = *LS48MHZ: System clock is expected at 48 MHz, FS/LS USB clock divide-by is set to 8
                           01    = *CPUDIV: CPU system clock (48 MHz) divided by 2 (= 24 MHZ)
                             0   =  Unimplemented
                              x  =  CFGPLLEN: (ignored, set dynamically)
                               x =  PLLSEL: (ignored, set dynamically)
            CONFIG1H 28 00101000
                        0        =  IESO: Oscillator Switchover mode disabled
                         0       =  FCMEN: Fail-Safe Clock Monitor disabled
                          1      = *PCLKEN: Primary Clock is always enabled
                           0     =  Unimplemented
                            1000 = *FOSC: Internal oscillator block
            CONFIG2L 5F 01011111
                        0        =  Unimplemented
                         1       =  LPBOR: Low-Power Brown-out Reset disabled
                          0      =  Unimplemented
                           11    =  BORV: VBOR set to 1.9V nominal
                             11  =  BOREN: Brown-out Reset enabled in hardware only (SBOREN is disabled)
                               1 =  PWRTEN: Power-up Timer disabled
            CONFIG2H 3C 00111100
                        00       =  Unimplemented
                          1111   =  WDTPS: Watchdog Timer Postscale Select = 1:32768
                              00 =  WDTEN: Watchdog Timer disabled in hardware, SWDTEN bit disabled
            CONFIG3L 00 00000000
                        00000000 =  Unimplemented
            CONFIG3H 53 01010000
                        0        = *MCLRE: RE3 input pin enabled; MCLR disabled
                         1       =  SDOMX: SDO is on RB3
                          0      =  Unimplemented
                           1     =  T1CMX: T1CKI is on RC0
                            00   =  Unimplemented
                              0  =  PDADEN: ANSELB<5:0> resets to 0, PORTB<4:0> pins are configured as digital I/O on Reset
                               0 =  CCP2MX: CCP2 input/output is multiplexed with RB3
            CONFIG4L 81 10000001 =  Default
            CONFIG5L 0F 00001111 =  Default
            CONFIG5H C0 11000000 =  Default
            CONFIG6L 0F 00001111 =  Default

OPERATION - Plug it in.

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

  usbToHost[0] = REPORT_ID_KEYBOARD;     // Report Id = Keyboard
  usbToHost[1] = 0;                      // No modifiers
  usbToHost[2] = 0;                      // Reserved for OEM
  usbToHost[3] = 0;                      // No key pressed
  bUSBReady = FALSE;
  while (!bUSBReady)
  {
    HID_Enable(&usbFromHost, &usbToHost);
    for (i=0; !bUSBReady && i < 50; i++) // Keep trying for up to 50 x 100 ms = 5 seconds
    {
      Delay_ms(100);
      CAPSLOCK_LED = ON;
      bUSBReady = HID_Write(&usbToHost, 4) != 0; // Copy to USB buffer and try to send
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
//  ANSELB = 0b00000000;    // Configure all PORTB bits as digital
  ANSELC = 0b00000000;    // Configure all PORTC bits as digital

  LATA = 0;
//  LATB = 0;
  LATC = 0;

  //        76543210
  TRISA = 0b00000000;     // 1=Input, 0=Output
//  TRISB = 0b01111000;
  TRISC = 0b00110000;

  OSCCON = 0b01110000;
//           x              0  = IDLEN: Sleep on SLEEP instruction
//            xxx           111= IRCF: HFINTOSC (16 MHz)
//               x          0  = OSTS: Status bit
//                x         0  = HFIOFS: Status bit
//                 xx       00 = SCS: Primary clock (determined by FOSC in CONFIG1H)
  while (!HFIOFS_bit);    // Wait for HFINTOSC to stabilise
  while (!OSTS_bit);      // Wait until FOSC<3:0> is being used

// You can enable the PLL and set the PLL multipler two ways:
// 1. At compile time - setting configuration fuses: CFGPLLEN and PLLSEL
//    ...but only if you use an External Clock (EC).
// 2. At run time - setting OSCCON2.PLLEN and OSCTUNE.SPLLMULT
//    ...but only if you use a High Speed crystal (HS), or
//       the High Frequency Internal Oscillator (HFINTOSC).
// This project uses the HFINTOSC set to run at 16 MHz and the only valid
// PLL multiplier for USB is x3.
  SPLLEN_bit = 0;         // OSCCON2.SPLLEN = 0:    Disable the PLL, then...
  SPLLMULT_bit = 1;       // OSCTUNE.SPLLMULT = 1: Set the PLL multiplier to x3
  SPLLEN_bit = 1;         // OSCCON2.SPLLEN = 1:    Now re-enable the PLL
  while(!PLLRDY_bit);     // Wait for PLL to lock

  ACTEN_bit = 0;          // Disable Active Clock Tuning, then...
  ACTSRC_bit = 1;         // Tune HFINTOSC to match USB host clock
  ACTEN_bit = 1;          // Enable Active Clock Tuning
//  while (!ACTLOCK_bit);   // Wait until HFINTOSC is successfully tuned

  cFlags = 0;             // Reset all flags

  // Set up USB
  UPUEN_bit = 1;          // USB On-chip pull-up enable
  FSEN_bit = 1;           // 1 = USB Full Speed enabled (requires 48 MHz USB clock)
                          // 0 = USB Full Speed disabled (requires 6 MHz USB clock)

// Timer3 is used for human-scale delays (Timer3 is not always enabled)
  T1CON   = 0b00110010;
//            xx             00 = TMR1CS: Timer3 clock source is instruction clock (Fosc/4)
//              xx           11 = TMR1PS: Timer3 prescale value is 1:8
//                x          0  = SOSCEN: Secondary Oscillator disabled
//                 x         0  = T1SYNC: Ignored because TMR1CS = 0x
//                  x        1  = RD16:   Enables register read/write of Timer3 in one 16-bit operation
//                   x       0  = TMR1ON: Timer3 is off
// Timer3 tick rate = 48 MHz FOSC/4/8 = 1.5 MHz (667 ns)
// Timer3 interrupt rate = 1.5 MHz / 65536 = 22.9 times per second

  CAPSLOCK_LED = OFF;

//----------------------------------------------------------------------------
// Let the interrupts begin
//----------------------------------------------------------------------------

  TMR1IE_bit = 1;         // Enable Timer3 interrupts (for keepalive)
  IOCIE_bit = 1;          // Enable PORTB/C Interrupt On Change interrupts
  PEIE_bit = 1;           // Enable peripheral interrupts
  GIE_bit = 1;            // Enable global interrupts

  enableUSB();            // Enable USB interface
}




void main()
{
  Prolog();
  nRemainingTimerTicks = INTERVAL_IN_SECONDS(10);
  TMR1ON_bit = 1;   // Enable keepalive interrupts
  while (1)
  {
    if (bUSBReady)
    {
      if (HID_Read() == 2 && usbFromHost[0] == REPORT_ID_KEYBOARD)   // If a host LED indication response is available
      {
        leds.byte = usbFromHost[1];   // Remember the most recent LED status change
        if (leds.bits.CapsLock)
          CAPSLOCK_LED = ON;
        else
          CAPSLOCK_LED = OFF;
      }
      if (!nRemainingTimerTicks)      // If the keepalive timer has popped
      {
        CAPSLOCK_LED ^= 1;            // Flash LED to indicate a keepalive is being sent
        usbToHost[1] = 0b10111011;    // Right(GUI,SHIFT,CTL)+Left(GUI,SHIFT,CTL) <-- a very unusual combination requiring 6 simultaneous finger presses
        bUSBReady = HID_Write(&usbToHost, 4) != 0; // Send a "no key pressed" signal to the host
        usbToHost[1] = 0b00000000;    // No key pressed
        bUSBReady = HID_Write(&usbToHost, 4) != 0; // Send a "no key pressed" signal to the host
        Delay_ms(10);                 // Keep the LED on or off for a detectable amount of time
        CAPSLOCK_LED ^= 1;            // Restore the original CAPSLOCK light state
        nRemainingTimerTicks = INTERVAL_IN_SECONDS(10);
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
  USB_Interrupt_Proc();        // Always give the USB module first opportunity to process
  if (TMR1IF_bit)              // Timer3 interrupt? (22.9 times/second)
  {
    nRemainingTimerTicks--;    // Decrement delay count
    TMR1IF_bit = 0;            // Clear the Timer3 interrupt flag
  }
}