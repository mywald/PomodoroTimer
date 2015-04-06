#include "picsetup.h"
#include "framework.h"
#include "sevensegment.h"

uchar cycles;
uchar second = 15; //30,637254901960784/2  == timer interrupts for one second

uchar tomato_time = 3*60;
uchar pause_time = 2*60;
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
        displayCharAsDecimal((current_time + 59) / 60);
    } else {
        displayCharAsDecimal(current_time);
    }
}

void interrupt ISR() {
    if (TMR2IF) {
        TMR2ON = 0; //disable timer
        
        cycles++;
        if (cycles == second) {
            one_second_passed();
            cycles = 0;   
            delayms(85); // correct rounding error for second counting
        }
        
        //restart timer, reset interrupt
        TMR2IF = 0;
        TMR2 = 0;
        TMR2ON = 1;
    }
}


void main(void) {
    configureports();
 
    //init businessdata
    tomatomode = 0; //start with pause
    current_time = 6; //6 seconds to go
    one_second_passed();
    
    //start timer and interrupts
    T2CON = 0b01111011;  //Setup Timer2 with Postscaler=1:16 and Prescaler=16
    PR2 = 255;           //Timer overflow max
    TMR2IF = 0;
    TMR2IE = 1;
    PEIE = 1;
    GIE = 1;
    TMR2ON = 1;
    
            
    while(1) {
        displayCharAsDecimal(88);
        NOP();
    }

}

