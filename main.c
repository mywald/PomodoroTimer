#include "framework.h"
#include "picsetup.h"
#include "sevensegment.h"
#include "nec_ir_driver.h"


#define TOMATO_TIME 1500  //25 Minuten
#define PAUSE_TIME 600   //10 Minuten

#define CYCLES_PER_SECOND 15  //30,637254901960784/2  == timer interrupts for one second
#define MISSING_MS_PER_SECOND 15  //to correct rounding error

#define BUTTON_MENU 0b11001000
#define BUTTON_VOL_UP 0b00101000
#define BUTTON_VOL_DOWN 0b10101000
#define BUTTON_MUTE 0b01001000
#define BUTTON_POWER 0b00011000


uint cycles;
bit tomatomode = 0;
uint current_time = 0;
bit pausing = 0;

void switch_mode();
void one_second_passed();
void init_businessdata();
void start_timer_and_interrupts();
void handle_remote_button();
void refresh_view();


void main(void) {
    configureports();
    ir_setup();
    init_businessdata();
    start_timer_and_interrupts();

    while(1) {
        if (ir_isReady()){
            handle_remote_button();
        }
    }
}

void init_businessdata(){
    LED_GREEN=1;
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

    ir_interruptHandler();
}

void one_second_passed(){
    if (pausing){
        if (tomatomode) {
            LED_RED = !LED_RED;
        } else {
            LED_GREEN = !LED_GREEN;
        }
        LED_DOOR = 0;
    } else {
        current_time--;

        if (current_time == 0){
            switch_mode();
        }
    }
    
    refresh_view();
}


void switch_mode(){
    tomatomode = !tomatomode;
    
    if (tomatomode) {
        current_time = TOMATO_TIME;
    } else {
        current_time = PAUSE_TIME;
    }
    
    refresh_view();
}

void refresh_view(){
    if (tomatomode) {
        LED_DOOR = !LED_DOOR;
        LED_RED = 1;
        LED_GREEN = 0;
    } else {
        LED_GREEN = 1;
        LED_RED = 0;
        LED_DOOR = 0;
    }

    if (current_time > 60){
        displayCharAsDecimal((current_time + 59) / 60);
    } else {
        displayCharAsDecimal(current_time);
    }
}

void handle_remote_button(){
    uint8 cmd = ir_popLatestCommand();
    
    if (cmd == BUTTON_MENU){
        switch_mode();
    } else if (cmd == BUTTON_VOL_UP){
        current_time += 61;
        current_time += 60 - (current_time % 60);
    } else if (cmd == BUTTON_VOL_DOWN){
        if (current_time > 60) {
            current_time -= 60;
            current_time += 60 - (current_time % 60);
        } else {
            switch_mode();
        }
    } else if (cmd == BUTTON_POWER){
        //standby
    } else if (cmd == BUTTON_MUTE){
        pausing = !pausing;
    } else {
        displayByteOnLED(cmd);
    }

    refresh_view();
}
