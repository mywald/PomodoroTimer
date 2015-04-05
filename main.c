#include "picsetup.h"
#include "framework.h"
#include "sevensegment.h"
#include "rfm12.h"

#define MODE_RECEIVE 0x01
#define MODE_REGISTRATION 0x02
#define BUFFERSIZE 16

#define COMMAND_ON 0x72
#define COMMAND_OFF 0x75
#define COMMAND_REQUEST_STATUS 0x51
#define COMMAND_REGISTER_CHANNEL 0x61


uchar receivebuffer[BUFFERSIZE] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uchar workingmode = MODE_RECEIVE;
uchar channelflags[5] = {0, 0, 0, 0, 0};

void refreshChannels() {
    RELAIS_PORT_0 = channelflags[0];
    RELAIS_PORT_1 = channelflags[1];
    RELAIS_PORT_2 = channelflags[2];
    RELAIS_PORT_3 = channelflags[3];
}

void handle_incoming_command() {
    //case which command has been sent:
    //1. toggle output of certain port
    //2. schedule status callback in some delay, to send information of all ports
    uchar device = receivebuffer[0];
    uchar channel = receivebuffer[1];
    uchar command = receivebuffer[2];

    if (command == COMMAND_REGISTER_CHANNEL & SWITCH){

    }

    
    //TODO: compare if this chip is addressed
    if (true){
        if (command == COMMAND_REQUEST_STATUS){
            delayms(100);

            uchar param[3] = {0, 0, 0};
            param[0] = device;
            param[1] = channel;

            if (channelflags[channel]) {
                param[2] = COMMAND_ON;
            } else {
                param[2] = COMMAND_OFF;
            }

            rfm12_send_data(param);
        } else if (command == COMMAND_ON) {
            channelflags[channel] = true;
            refreshChannels();
        } else if (command == COMMAND_OFF) {
            channelflags[channel] = false;
            refreshChannels();
        }
    }
    
    rfm12_init_receiver();
}

void handle_incoming_stream() {
    uchar receivebufferpos = 0;
    uchar data = 0;
    uint waited = 0;
            
    while( data != COMMAND_END_OF_STREAM && receivebufferpos < BUFFERSIZE && waited < 5000) {
        
        struct Rfm12Status status = rfm12_read_status();
        if (status.isFifoFull) {
            data = rfm12_read_data();
            
            if (data == COMMAND_END_OF_STREAM) {
                handle_incoming_command();
                return;
            } 

            receivebuffer[receivebufferpos] = data;
            receivebufferpos++;
        }
    
        while (!RA2) {
            delayms(1);
            waited++;
        }
    }

}

void interrupt ISR() {
   GIE=0;                //No overlapping interrupts, so they are disabled

   if (INTF) {      //Check if it is RA2 external Interrupt (= RFM12 IRQ)
       LED = 1;
       handle_incoming_stream();
       rfm12_wait_for_new_stream();
   }

   INTF=0;
   GIE=1;                //Enable interrupts again
   
}


void main(void) {

    configureports();
    refreshChannels();

    rfm12_init_common();
    rfm12_init_receiver();
    delayms(50);
    
    INTF = 0;
    //GIE = 1;
    LED = 1;
    LED2 = 0;
    
    rfm12_wait_for_new_stream();

    while(1) {
         for (uchar i = 0; i < 5; i++) {
/*             if (channelflags[i]){
                displayChar(i | 0x10);
             } else {
                displayChar(i);
             }*/
             
                displayChar(0x07);

             if (SWITCH){

                uchar param[3] = {0, 0, 0};
                param[0] = 0xAA;
                param[1] = 0x02;
                param[2] = COMMAND_ON;

                rfm12_send_data(param);
             }
                LED = !LED;
                LED2 = !LED2;
                LED3 = !LED3;
             delayms(700);
         }
    }

}

