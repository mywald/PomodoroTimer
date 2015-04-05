#include "sevensegment.h"

uchar characters[16] = { 0b11001111, 0b00001100, 0b10101011, 0b10101110, 0b01101100, 0b11100110,
                         0b11100111, 0b11001100, 0b11101111, 0b11101110, 0b11101101, 0b01100111,
                         0b00100011, 0b00101111, 0b11100011, 0b11100001 };


void displayCharAsDecimal(uchar val){
    uchar hnib = ((val & 0xF0) >> 4);
    uchar lnib = val & 0x0F;

    if (lnib > 9){
        lnib=lnib-10;
        hnib++;
    }
    
    DISPLAY_SLAVE_SELECT = 1;
    SSPBUF = characters[lnib];
    while (!SSPSTATbits.BF);
    SSPBUF = characters[hnib];
    while (!SSPSTATbits.BF);
    DISPLAY_SLAVE_SELECT = 0;
}

