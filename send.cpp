/*
 Usage: ./send <command>
 Command is 0 for OFF and 1 for ON
 */

#include "RCSwitch.h"
#include "clickButton.h"
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>

// "addresses" for the relays
#define DILLON_ON   0x451533
#define DILLON_OFF  0x45153C

#define SARA_ON     0x4515C3
#define SARA_OFF    0x4515CC

#define LIQUOR_ON   0x451703
#define LIQUOR_OFF  0x45170C

// length of each bit
#define PULSE_LENGTH 190     // 0xBE

// length of packet to send
#define BIT_LENGTH 24

// the lamp's state is stored in a two bit binary number
const int DILLON_BIT  = 0b01; // bit 0 is the state of Dillon's lamp
const int SARA_BIT  = 0b10;   // bit 1 is the state of Sara's lamp
int lampState = 0b00;   // stores the state of both lamps

void toggleDillon(RCSwitch mySwitch);
void toggleSara(RCSwitch mySwitch);

int main(int argc, char *argv[]) {

    // pin 3 is really GPIO 22 on the Pi
    int PIN = 3;            // 433 Mhz transmitter
    int dillonLamp = 21;    // Dillon's lamp switch
    int saraLamp = 23;      // Sara's lamp switch
    int command  = atoi(argv[1]);

    if (wiringPiSetup () == -1) return 1;
	printf("sending command[%i]\n", command);
	RCSwitch mySwitch = RCSwitch();
    // setup the transmitter on pin 3
	mySwitch.enableTransmit(PIN);

    // Set pulse length of a bit
    mySwitch.setPulseLength(PULSE_LENGTH);

    // set up the Click Button object
    ClickButton dillonButton(dillonLamp, LOW, CLICKBTN_PULLUP);
    ClickButton saraButton(saraLamp, LOW, CLICKBTN_PULLUP);

    // set up click button click counter
    volatile int dillonClicks = 0;
    volatile int saraClicks = 0;

    // set up the 2 switch pins as inputs with pullups
    pinMode(dillonLamp, INPUT);
    pullUpDnControl(dillonLamp, PUD_UP);
    pinMode(saraLamp, INPUT);
    pullUpDnControl(saraLamp, PUD_UP);

    while(1) {
        // Update Dillon's button state
        dillonButton.Update();

        // Dillon's button was clicked
        if (dillonButton.clicks != 0) dillonClicks = dillonButton.clicks;

        if(dillonClicks == 1){
            printf("SINGLE click\n");
            //blockInterrupt = //blockInterruptTimes;
            toggleDillon(mySwitch);
        }

        if(dillonClicks == 2){
            printf("DOUBLE click\n");
            //blockInterrupt = //blockInterruptTimes;
            toggleSara(mySwitch);
        }

        if(dillonClicks == -1){
            printf("SINGLE LONG click\n");
            //blockInterrupt = //blockInterruptTimes;
            // TODO
            //matchToggle("DILLON");
        }

        // reset counter for next round
        dillonClicks = 0;

        // update Sara's button state
        saraButton.Update();

        // Sara's button was clicked
        if (saraButton.clicks != 0) saraClicks = saraButton.clicks;

        if(saraClicks == 1){
            printf("SINGLE click\n");
            //blockInterrupt = //blockInterruptTimes;
            toggleSara(mySwitch);
        }

        if(saraClicks == 2){
            printf("DOUBLE click\n");
            //blockInterrupt = //blockInterruptTimes;
            toggleDillon(mySwitch);
        }

        if(saraClicks == -1){
            printf("SINGLE LONG click\n");
            //blockInterrupt = //blockInterruptTimes;
            // TODO
            // matchToggle("SARA");
        }

        // reset counter for next round
        saraClicks = 0;

        delay(5);
    }

    switch(command) {
        case 1:
            // turn liquor lights on
            mySwitch.send(LIQUOR_ON, BIT_LENGTH);
            break;
        case 0:
            // turn liquor lights off
            mySwitch.send(LIQUOR_OFF, BIT_LENGTH);
            break;
        default:
            printf("command[%i] is unsupported\n", command);
            return -1;
    }
	return 0;
}

/* toggle Dillon's lamp */
void toggleDillon(RCSwitch mySwitch){
    // toggle the state of Dillon's lamp
    lampState ^= DILLON_BIT;

    // send new state
    if ((lampState & DILLON_BIT) == DILLON_BIT){
        //turn on Dillon's lamp
        mySwitch.send(DILLON_ON, BIT_LENGTH);
    }
    else{
        // turn off Dillon's lamp
        mySwitch.send(DILLON_OFF, BIT_LENGTH);
    }
}

/* toggle Sara's lamp */
void toggleSara(RCSwitch mySwitch){
    // toggle the state of Sara's lamp
    lampState ^= SARA_BIT;

    // send new state
    if ((lampState & SARA_BIT) == SARA_BIT){
        // turn on Sara's lamp
        mySwitch.send(SARA_ON, BIT_LENGTH);
    }
    else{
        // turn off Sara's lamp
        mySwitch.send(SARA_OFF, BIT_LENGTH);
    }
}
