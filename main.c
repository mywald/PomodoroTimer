#include "picsetup.h"
#include "framework.h"
#include "sevensegment.h"


#define TOMATO_TIME 180  //3 Minuten
#define PAUSE_TIME 120   //2 Minuten

#define CYCLES_PER_SECOND 15  //30,637254901960784/2  == timer interrupts for one second
#define MISSING_MS_PER_SECOND 85  //to correct rounding error

uchar cycles;
bit tomatomode = 0;
uchar current_time = 0;

void switch_mode();
void one_second_passed();
void init_businessdata();
void start_timer_and_interrupts();



void main(void) {
    configureports();
    init_businessdata();
    start_timer_and_interrupts();

    while(1) {
        NOP(); //nothing to do since everything happens in interrupts
    }
}

void init_businessdata(){
    tomatomode = 0; //start with pause
    current_time = 6; //6 seconds to go
    one_second_passed();
}

void start_timer_and_interrupts(){
    T2CON = 0b01111011;  //Setup Timer2 with Postscaler=1:16 and Prescaler=16
    PR2 = 255;           //Timer overflow max
    TMR2IF = 0;
    TMR2IE = 1;
    PEIE = 1;
    GIE = 1;
    TMR2ON = 1;   
}

void interrupt ISR() {
    if (TMR2IF) { 
        TMR2ON = 0; //disable timer
        
        cycles++;
        if (cycles == CYCLES_PER_SECOND) {
            one_second_passed();
            cycles = 0;   
            delayms(MISSING_MS_PER_SECOND); // correct rounding error for second counting
        }
        
        //restart timer, reset interrupt
        TMR2IF = 0;
        TMR2 = 0;
        TMR2ON = 1;
    }
}

void one_second_passed(){
    current_time--;
    
    if (tomatomode) {
        LED=!LED;
    }
    
    if (current_time == 0){
        switch_mode();
    }
    if (current_time > 60){
        displayCharAsDecimal((current_time + 59) / 60);
    } else {
        displayCharAsDecimal(current_time);
    }
}


void switch_mode(){
    tomatomode = !tomatomode;
    
    if (tomatomode) {
        current_time = TOMATO_TIME;

        LED=1;
        LED2=0;
        LED3=1;
    } else {
        current_time = PAUSE_TIME;

        LED=0;
        LED2=1;
        LED3=0;
    }
}

