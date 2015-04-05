#include "picsetup.h"

void configureports(){
    //Init IO
    ANSELH = 0X0;         //Disable analog Ports
    ANSEL = 0X0;          //Disable analog Ports

    TRISBbits.TRISB6 = 0; //RB6=Clock as Output
    TRISBbits.TRISB4 = 1; //RB4=Serial Data In as Input
    TRISCbits.TRISC7 = 0; //RC7=Serial Data Out as Output
    TRISCbits.TRISC2 = 0; //RC2=Slave Select for RFM12 as Output
    TRISCbits.TRISC1 = 0; //RC1=LED as Output
    TRISBbits.TRISB5 = 0; //RB5=LED2 as Output
    TRISCbits.TRISC4 = 0; //RC3=LED3 as Output
    TRISCbits.TRISC0 = 0; //RC0=Slave Select for Display as Output

    //Init SPI
    SSPEN = 0;           //Turn off SPI to make it configurable
    SSPSTAT= 0b01000000; //SMP=0, CKE=1,
    SSPCON = 0b00000010; //Enable SPI = false, CKP = 0, Configure as Master with CLK = FOSC/64
    SSPEN = 1;           //Turn on SPI
    SSPSTATbits.BF = 1;

    nSPI_SLAVE_SELECT = 1;
    DISPLAY_SLAVE_SELECT = 0;
    LED = 0;
    LED2 = 0;
    LED3 = 0;
}
