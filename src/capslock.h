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
#define OVERHEAD 8
#define INTERVAL_IN_SECONDS(n) (CLOCK_FREQUENCY / 4 / TIMER3_PRESCALER / 65536 * n + OVERHEAD)


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

#define CAPSLOCK_LED         RA0_bit

volatile uint8_t nRemainingTimerTicks;
volatile uint8_t             cFlags;
#define bUSBReady            cFlags.B0

// USB buffers must be in USB RAM, hence the "absolute" specifier...
uint8_t BANK4_RESERVED_FOR_USB[256] absolute 0x400; // Prevent compiler from allocating
                                                    // RAM variables in Bank 4 because
                                                    // the USB hardware uses that bank.
                                                    // Refer to the PIC18F25K50 datasheet
                                                    // section "6.4.1 USB RAM" for more
                                                    // information.
uint8_t usbFromHost[1+1] absolute 0x500;  // Buffer for PIC <-- Host (ReportId + 1 byte)
uint8_t usbToHost[1+3]   absolute 0x508;  // Buffer for PIC --> Host (ReportId + 3 bytes)