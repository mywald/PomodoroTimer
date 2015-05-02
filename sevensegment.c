#include "sevensegment.h"

uchar characters_l[10] = { 0b11111010, 0b00110000, 0b01011110, 0b00111110, 0b10110100, 0b10101110,
                         0b11101110, 0b00111000, 0b11111110, 0b10111110 };

uchar characters_h[10] = { 0b01111110, 0b01010000, 0b01001111, 0b01011011, 0b01110001,  0b00111011,
                         0b00111111, 0b01010010, 0b01111111, 0b01111011 };

uchar currently_displayed = 0xFF;

void displayCharAsDecimal(uchar val){
    if (currently_displayed == val){
        return;
    }
    currently_displayed = val;
    
    uchar hnib = 0;
    uchar lnib = 0;

    while (val > 9){
        val=val-10;
        hnib++;
    }
    lnib = val;
    
    DISPLAY_SLAVE_SELECT = 1;
    SSPBUF = characters_l[lnib];
    while (!SSPSTATbits.BF);
    SSPBUF = characters_h[hnib];
    while (!SSPSTATbits.BF);
    DISPLAY_SLAVE_SELECT = 0;
}

void displaySingleDot(){
    DISPLAY_SLAVE_SELECT = 1;
    SSPBUF = 0b000000001;
    while (!SSPSTATbits.BF);
    SSPBUF = 0b000000000;
    while (!SSPSTATbits.BF);
    DISPLAY_SLAVE_SELECT = 0;
}
