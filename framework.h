#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "picsetup.h"

/******************************************************************************/
/* Types                                                                      */
/******************************************************************************/

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned char bool;
typedef signed char sint8;
typedef signed int sint16;
typedef signed long sint32;
typedef unsigned char  uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;

#define FALSE    0
#define TRUE    1


/******************************************************************************/
/* Tools                                                                      */
/******************************************************************************/

/* bit access macros */
#define bit_set(value, bitpos)        ((value) |= ((uint16)1 << (bitpos)))
#define bit_clear(value, bitpos)    ((value) &= (~((uint16)1 << (bitpos))))
#define bit_test(value, bitpos)        ((((value) >> (bitpos)) & 0x1) != 0)


void delayms(uint ms);

void fastBlink();

void displayByteOnLED(uchar answer);

uchar EEread(uchar address);

void EEwrite(uchar address, uchar value);


#endif //FRAMEWORK_H