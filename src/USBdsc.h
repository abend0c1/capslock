#define LO(x) ((uint8_t) (x))
#define HI(x) ((uint8_t) ((uint16_t)(x)) >> 8)
#define WORD(x) LO(x), HI(x)


#define REPORT_DESCRIPTOR(array) \
uint16_t t##array[] = {STRING}; /* This array will be optimised away because it is never referenced at execution time. */ \
                                /* It is used only to compute the length of the string specified by the STRING macro.  */ \
const struct  \
{ \
  uint8_t report[sizeof t##array / sizeof t##array[0]]; \
} array = \
  { \
    {STRING} \
  }


#define STRING_DESCRIPTOR(array) \
uint16_t t##array[] = {STRING}; /* This array will be optimised away because it is never referenced at execution time. */ \
                             /* It is used only to compute the length of the string specified by the STRING macro.  */ \
const struct \
{ \
  uint8_t  bLength; \
  uint8_t  bDscType; \
  uint16_t string[sizeof t##array / sizeof t##array[0]]; \
} array = \
  { \
    2+sizeof t##array, /* bLength:  Size of this structure in bytes (including bLength and bDscType fields) */ \
    0x03,              /* bDscType: 0x03 means this is a String descriptor */ \
    {STRING}           /* string:   This is the array of 2-byte "characters" comprising the string */ \
  }