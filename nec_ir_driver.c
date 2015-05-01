/*
 * IR decoder driver
 * for NEC Code
 *                                                                           
 * The following driver is prepared to use pin RA5 and TMR0.
 * 
 */
#include "nec_ir_driver.h"

/* Timing setup: ticks per bit type - have to be adapted if timer is faster */
#define IR_START_PERIOD_LO          140  //9ms
#define IR_START_PERIOD_HI           69  //4.5ms
#define IR_PAUSE                      9  //
#define IR_LOW_BIT                    9
#define IR_HIGH_BIT                  25

/* Operation modes: */
#define IR_WAIT_FOR_START        0
#define IR_WAIT_FOR_START_HI     1
#define IR_WAIT_FOR_DATA         2
#define IR_IS_READING_DATA       3
#define IR_IS_PAUSING            4

/* status variables */
static uint8 ir_received_code = 0;  
static uint32 ir_temp_code = 0;  
static uint8 ir_bit_pos = 0;
static uint8 ir_operationmode;
static bool ir_ready = FALSE;
static bool ir_pinstate_old = TRUE;

bool ir_isReady(){
    return ir_ready;
}

uint8 ir_popLatestCommand(){
    ir_ready = false;
    return ir_received_code;
}

void ir_setup() {
    //Timer0 Registers Prescaler= 64 - TMR0 Preset = 0 - Freq = 61.04 Hz - Period = 0.016384 seconds = 64uS pro Tick
    //Setup for PIC which is per default set to 4MHz (e.g. PIC16F690)
    T0CS = 0;  // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
    T0SE = 0;  // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
    PSA = 0;   // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the Timer0
    PS2 = 1;   // bits 2-0  PS2:PS0: Prescaler Rate Select bits
    PS1 = 0;
    PS0 = 1;

    TMR0 = 0;  // preset timer register
    T0IE = 0;    //disable TMR0 Interrupt
    
    //Configure RA5 as Interrupt for TSOP
    TRISAbits.TRISA5 = 1; //Enable Input (remember to have Analog Functions disabled ANSEL and ANSELH)
    IOCA5 = 1;            //Make RA5 to accept Interrupts
    
    RABIF = 0;            //Interrupt Flags reset
    INTF = 0;             //Interrupt Flags reset
    RABIE = 1;            //Enable Port-A/B-Change Interrupts

    GIE = 1;
}

void ir_abort() {
    T0IF=0; //disable timer interrupt
    T0IE=0;
    ir_operationmode = IR_WAIT_FOR_START; //set operation mode to default
}

void ir_messageReceived() {
    ir_abort();

    //first 2 bytes = manufacturer code (and inversed)
    //second 2 bytes is command (and inversed command)
    uint8 command = (ir_temp_code & 0x0000FF00)>>8;
    uint8 command_inv = ir_temp_code & 0x000000FF;
            
    if (command ^ command_inv) {  //check if data is valid
        ir_received_code = command;
        ir_ready = TRUE;
    }
}

bool ir_isInRange(uint8 units, uint8 time){
    return (units - 4)  < time && time < (units + 4);
}

bool ir_fallingEdge(bool pin){
    return ir_pinstate_old && !pin;
}

bool ir_risingEdge(bool pin){
    return !ir_pinstate_old && pin;
}

void ir_interruptHandlerForPinChange() {
    bool current_pin;
    uint8 tdiff;
    
    // get current pin status
    current_pin = RA5;  //use another pin if required
    
    if (current_pin == ir_pinstate_old) {
        return; //interrupt has not been called due to a change of our pin
    }
    
    //calculate time difference to last interrupt call
    tdiff = TMR0;  //use another timer if required
    TMR0 = 0;
    if (ir_operationmode == IR_WAIT_FOR_START) {
        if (ir_fallingEdge(current_pin)) {
            T0IF=0;
            T0IE=1;//TMR0 Interrupt enabled

            ir_temp_code = 0;
            ir_bit_pos = 32;
            
            ir_operationmode = IR_WAIT_FOR_START_HI;
        }
    } else if (ir_operationmode == IR_WAIT_FOR_START_HI) {
        if (ir_risingEdge(current_pin)) {
            if (ir_isInRange(IR_START_PERIOD_LO, tdiff )) {
                ir_operationmode = IR_WAIT_FOR_DATA;
            }
        }
    } else if (ir_operationmode == IR_WAIT_FOR_DATA) {
        if (ir_fallingEdge(current_pin)) {
            if (ir_isInRange(IR_START_PERIOD_HI, tdiff)) {
                ir_operationmode = IR_IS_PAUSING;
            }
        }
    } else if (ir_operationmode == IR_IS_PAUSING) {
        if (ir_risingEdge(current_pin)) {
            if (ir_isInRange(IR_PAUSE, tdiff )) {
                ir_operationmode = IR_IS_READING_DATA;
            }
            
            if (ir_bit_pos == 0){
                ir_messageReceived();
            }
        }
    } else if (ir_operationmode == IR_IS_READING_DATA) {
        if (ir_fallingEdge(current_pin)) {
            if (ir_isInRange(IR_HIGH_BIT, tdiff)) {
                ir_bit_pos--;
                bit_set(ir_temp_code, ir_bit_pos);
                ir_operationmode = IR_IS_PAUSING;
            } else if (ir_isInRange(IR_LOW_BIT, tdiff)) {
                ir_bit_pos--;
                bit_clear(ir_temp_code, ir_bit_pos);
                ir_operationmode = IR_IS_PAUSING;
            }
        }
    }
    
    ir_pinstate_old = current_pin;  //save current pin state for edge detection
}

void ir_interruptHandler() {
    if (RABIF) {          //Check if it is port A/B change Interrupt (= TSOP IRQ)
       RABIF=0;
       ir_interruptHandlerForPinChange();
    } else if (T0IF) {
       T0IF=0;
       ir_abort();
    }
}
