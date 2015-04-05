#include "picsetup.h"
#include "framework.h"
#include "sevensegment.h"

uchar cycles;
//30,637254901960784
uchar minute = 919;
uchar second = 15;

uchar tomato_time = 2*60;
uchar pause_time = 1*60;
bit tomatomode = 0;
uchar current_time = 0;


void switch_mode(){
    tomatomode = !tomatomode;
    
    if (tomatomode) {
        current_time = tomato_time;

        LED=1;
        LED2=0;
        LED3=1;
    } else {
        
        current_time = pause_time;
        LED=0;
        LED2=1;
        LED3=0;
    }
}

void one_second_passed(){
    current_time--;
    if (current_time == 0){
        switch_mode();
    }
    if (current_time > 60){
        displayCharAsDecimal((current_time + 60) / 60);
    } else {
        displayCharAsDecimal(current_time);
    }
}

void interrupt ISR() {
    if (TMR2IF) {
        cycles++;
        if (cycles == second) {
            one_second_passed();
            cycles = 0;      
        }
        TMR2IF = 0;
    }
}


void main(void) {
    configureports();
 
    //init businessdata
    tomatomode = 0;
    current_time = 1;
    one_second_passed();
    
    //start timer and interrupts
    PR2 = 255;
    T2CON = 0b01111011;  //Setup Timer2 with Postscaler=1:16 and Prescaler=16

    TMR2IF = 0;
    TMR2IE = 1;
    PEIE = 1;
    GIE = 1;
    TMR2ON = 1; //Timer2 = on
    
            
    while(1) {
        NOP();
    }

}

