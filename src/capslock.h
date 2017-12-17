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

#define CAPSLOCK_LED         LATA4_bit

#define SCROLL_LOCK_KEY      0x47

volatile uint16_t nRemainingTimerTicks;
volatile uint8_t             cFlags;
#define bUSBReady            cFlags.B0
#define bKeepAlive           cFlags.B1

uint8_t usbFromHost[1];
uint8_t usbToHost[1+1+6]; // 1 byte modifiers, 1 byte pad, 6 keys