#ifndef DIGITS_DISPLAY_H
#define DIGITS_DISPLAY_H
#include "bastypes.h"

extern void outRefToDIGITS(u16 ref1, u16 ref2);
extern void outDigitsToSPI();
extern u16 SPI_DATA[8];

#endif