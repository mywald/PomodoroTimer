#include "framework.h"


void delayms(uint ms){
    uint waitedms = 0;
    while (waitedms < ms){
        for(uint cycles = 0; cycles< 65; cycles++){
            NOP();
        }
        waitedms++;
    }
}



void fastBlink(){
   LED = 1;
   delayms(50);
   LED = 0;
   delayms(50);
   LED = 1;
   delayms(50);
   LED = 0;
   delayms(50);
   LED = 1;
   delayms(50);
   LED = 0;
}

void displayByteOnLED(uchar answer){
    uchar n = 8;
    while (n--){
        LED = 0;
        delayms(400);
        if (answer & 0b10000000) {
            LED = 1;
        }
        delayms(100);
        answer = answer << 1;
    }
    delayms(1000);
    LED = 0;
}

//Funktion um Daten aus dem EEPROM zu lesen
uchar EEread(uchar address){
    unsigned char gie_old=INTCONbits.GIE;
    INTCONbits.GIE=0;
    EECON1bits.EEPGD=0;                                 //EEprom Zugriff
    EEADR=address;                                      //Adresse setzen
    EECON1bits.RD=1;                                    //Lesezugriff
    INTCONbits.GIE=gie_old;
    return EEDATA;
}

//Funktion um Daten ins EEPROM zu schreiben
void EEwrite(uchar address, uchar value){
    unsigned char gie_old=INTCONbits.GIE;
    INTCONbits.GIE=0;
    EECON1bits.EEPGD=0;                                 //EEprom Zugriff
    EECON1bits.WREN=1;                                  //Schreibzugriff
    EEADR=address;                                      //Adresse nach Adressregister
    EEDATA=value;                                       //Daten ins Datenregister
    EECON2=0x55;                                        //Sicherheitscode (Datenblatt!!!)
    EECON2=0xAA;                                        //Sicherheitscode (Datenblatt!!!)
    EECON1bits.WR=1;                                    //schreiben
    while(EECON1bits.WR);                               //Auf Schreibende warten
    EECON1bits.WREN=0;                                  //Schreiben deaktivieren
    PIR2bits.EEIF=0;                                    //Write-Interrupt EEPROM löschen
    INTCONbits.GIE=gie_old;
}