#include "framework.h"
#include "picsetup.h"
#include "sevensegment.h"
#include "nec_ir_driver.h"

#define ADDRESS_TOMATOTIME 0x00
#define ADDRESS_PAUSETIME 0x01
#define DEFAULT_TOMATO_TIME_MINUTES 25
#define DEFAULT_PAUSE_TIME_MINUTES 10

#define CYCLES_PER_SECOND 15  //30,637254901960784/2  == timer interrupts for one second
#define MISSING_MS_PER_SECOND 15  //to correct rounding error

#define BUTTON_MENU 0b11001000
#define BUTTON_VOL_UP 0b00101000
#define BUTTON_VOL_DOWN 0b10101000
#define BUTTON_MUTE 0b01001000
#define BUTTON_POWER 0b00110000
#define BUTTON_APS 0b10010000


uint cycles;
bit tomatomode = 0;
uint current_time = 0;
bit pausing = 0;
bit setup = 0;
bit sleep = 0;
uint time_before_setup = 0;

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

void init_businessdata() {
    uchar storedtime = EEread(ADDRESS_TOMATOTIME);
    if (storedtime == 0x00 || storedtime == 0xFF){
        EEwrite(ADDRESS_TOMATOTIME, DEFAULT_TOMATO_TIME_MINUTES);
    }
    storedtime = EEread(ADDRESS_PAUSETIME);
    if (storedtime == 0x00 || storedtime == 0xFF){
        EEwrite(ADDRESS_PAUSETIME, DEFAULT_PAUSE_TIME_MINUTES);
    }
    
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
    if (T0IF && sleep) {
        SLEEP();
    }
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
        refresh_view();
    }
}


void switch_mode(){
    tomatomode = !tomatomode;
    
    if (tomatomode) {
        current_time = EEread(ADDRESS_TOMATOTIME)*60;
    } else {
        current_time = EEread(ADDRESS_PAUSETIME)*60;
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
    
    if (cmd == BUTTON_POWER){
        if (!sleep) {
            sleep = 1;
            TMR2ON = 0;
            LED_DOOR = 0;
            LED_RED = 0;
            LED_GREEN = 0;
            clearDisplay();
            SLEEP();
        } else {
            init_businessdata();
            TMR2ON = 1;
            sleep = 0;
        }
    } else if (sleep) {
        SLEEP();
    } else if (cmd == BUTTON_MENU){
        switch_mode();
    } else if (cmd == BUTTON_VOL_UP){
        if (current_time % 60 > 0) {
            current_time += 120 - (current_time % 60);
        } else {
            current_time += 60;
        }
    } else if (cmd == BUTTON_VOL_DOWN){
        if (current_time > 60) {
            if (current_time % 60 > 0) {
                current_time -= current_time % 60;
            } else {
                current_time -= 60;
            }
        } else {
            switch_mode();
        }
    } else if (cmd == BUTTON_MUTE){
        pausing = !pausing;
    } else if (cmd == BUTTON_APS){
        if (!setup) {
            time_before_setup = current_time;
            if (tomatomode) {
                current_time = EEread(ADDRESS_TOMATOTIME)*60;
            } else {
                current_time = EEread(ADDRESS_PAUSETIME)*60;
            }
            setup = 1;
            pausing = 1;
        } else {
            if (tomatomode) {
                EEwrite(ADDRESS_TOMATOTIME, current_time/60);
            } else {
                EEwrite(ADDRESS_PAUSETIME, current_time/60);
            }
            current_time = time_before_setup;
            pausing = 0;
            setup = 0;
        }
    } /*else {
        GIE=0;
        displayByteOnLED(cmd);
        GIE=1;
    }*/

    if (!sleep){
      refresh_view();
    }
}
