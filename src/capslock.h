#define VERSION "1.00"

#define OUTPUT        0
#define INPUT         1

#define ON            1
#define OFF           0

#define HIGH          1
#define LOW           0

#define ENABLE        1
#define DISABLE       0

#define TRUE  (1==1)
#define FALSE !TRUE

#define ELEMENTS(array) (sizeof(array)/sizeof(array[0]))
#define CLOCK_FREQUENCY       (__FOSC__ * 1000)
#define TIMER3_PRESCALER      8
#define INTERVAL_IN_SECONDS(n) (n * CLOCK_FREQUENCY / 4 / TIMER3_PRESCALER / 65536)


/*
    .---------------------------------------.
    |          REPORT_ID_KEYBOARD           | OUT: Report Id
    |---------------------------------------|
    |    |    |    |    |    |SCRL|CAPL|NUML| OUT: NumLock,CapsLock,ScrollLock - and 5 unused pad bits
    '---------------------------------------'
*/
typedef union
{
  uint8_t byte;
  struct
  {
    NumLock:1;      // xxxxxxx1
    CapsLock:1;     // xxxxxx1x
    ScrollLock:1;   // xxxxx1xx
    :5;             // Pad
  } bits;
} t_ledIndicators;

t_ledIndicators leds;

sbit BUTTON              at RA5_bit;
#define BUTTON_PRESSED   !BUTTON

#define CAPSLOCK_LED         LATA4_bit

#define SCROLL_LOCK_KEY      0x47

volatile uint16_t nRemainingTimerTicks;
volatile uint8_t             cFlags;
#define bUSBReady            cFlags.B0
#define bKeepAlive           cFlags.B1

// USB buffers must be in USB RAM, hence the "absolute" specifier...
// uint8_t RESERVE_BANK0_FOR_USB[80] absolute 0x020;  // 0x20-0x6F (0x50 bytes)
// uint8_t RESERVE_BANK1_FOR_USB[64] absolute 0x0A0;  // 0xA0-0xDF (0x40 bytes)
uint8_t usbFromHost[1+1]; //      absolute 0x0E0;  // 0xE0-0xE1 (0x02 bytes) Buffer for PIC <-- Host (ReportId + 1 byte)
uint8_t usbToHost[1+3]; //        absolute 0x0E8;  // 0xE8-0xEA (0x03 bytes) Buffer for PIC --> Host (ReportId + 3 bytes)