#include "RCSwitch/RCSwitch.h"
#include "clickButton/clickButton.h"
#include "pi-lamp.h"
#include "status.h"
#include <wiringPi.h>




// the lamp's state is stored in a two bit binary number
const int DILLON_BIT  = 0b01; // bit 0 is the state of Dillon's lamp
const int SARA_BIT  = 0b10;   // bit 1 is the state of Sara's lamp
int lampState = 0b00;   // stores the state of both lamps

// arguments for matchToggle
typedef enum {
    dillon,
    sara
} LampOwners;

void matchToggle(LampOwners owner, RCSwitch mySwitch);

void toggleDillon(RCSwitch mySwitch);
void toggleSara(RCSwitch mySwitch);

void toggleLight(void);




int main(void) {

    // pin 3 is really GPIO 22 on the Pi
    int PIN = 3;            // 433 Mhz transmitter
    int dillonLamp = 23;    // Dillon's lamp switch
    int saraLamp = 21;      // Sara's lamp switch

    if (wiringPiSetup () == -1) return 1;

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

    // create thread
    std::thread thread(scan_service);

    while(1) {
        // Update Dillon's button state
        dillonButton.Update();

        // Dillon's button was clicked
        if (dillonButton.clicks != 0) dillonClicks = dillonButton.clicks;

        if(dillonClicks == 1){
            //printf("SINGLE click\n");
            toggleDillon(mySwitch);
        }

        if(dillonClicks == 2){
            //printf("DOUBLE click\n");
            toggleSara(mySwitch);
        }

        if(dillonClicks == 3){
           //printf("TRIPLE click\n");
            toggleLight();
        }

        if(dillonClicks == -1){
            //printf("SINGLE LONG click\n");
            matchToggle(dillon, mySwitch);
        }

        // reset counter for next round
        dillonClicks = 0;

        // update Sara's button state
        saraButton.Update();

        // Sara's button was clicked
        if (saraButton.clicks != 0) saraClicks = saraButton.clicks;

        if(saraClicks == 1){
            //printf("SINGLE click\n");
            toggleSara(mySwitch);
        }

        if(saraClicks == 2){
            //printf("DOUBLE click\n");
            toggleDillon(mySwitch);
        }

        if(saraClicks == 3){
            //printf("TRIPLE click\n");
            toggleLight();
        }

        if(saraClicks == -1){
            //printf("SINGLE LONG click\n");
            matchToggle(sara, mySwitch);
        }

        // reset counter for next round
        saraClicks = 0;

        delay(5);
    }

    // kill the scanner and thread
    setStatus(kill);

    // RESOURCE ACQUISITION IS INITIALIZATION allows us to call detach()
    // in the case of exceptions
    ThreadRAII wrapperObj(thread);

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

/* set both lamps to a new state */
void switchLamps(boolean on, RCSwitch mySwitch){
    if(on){
        // turn both lamps on
        mySwitch.send(SARA_ON, BIT_LENGTH);
        mySwitch.send(DILLON_ON, BIT_LENGTH);
        lampState = 0b11;
    }
    else{
        // turn both lamps off
        mySwitch.send(DILLON_OFF, BIT_LENGTH);
        mySwitch.send(SARA_OFF, BIT_LENGTH);
        lampState = 0b00;
    }
}

/* set both lamps to the opposite of the button's lamp's current state */
void matchToggle(LampOwners owner, RCSwitch mySwitch){
    int buttonBit;

    // decide which button to check
    if (owner == dillon){
        buttonBit = DILLON_BIT;
    }
    else if (owner == sara){
        buttonBit = SARA_BIT;
    }

    // currently on, so turn both off
    if ((lampState & buttonBit) == buttonBit){
        switchLamps(false, mySwitch);
    }
    // currently off, so turn both on
    else {
        switchLamps(true, mySwitch);
    }
}

/* toggles the overhead light using a Switchmate */
void toggleLight(void){
    // need to kill scanner before connecting to switchmate w/ bluetooth
    setStatus(stop);

    // if on, turn off
    if (getSwitchState()){
        system("./Switchmate/off.sh");
    }
    // if off, turn on
    else {
        system("./Switchmate/on.sh");
    }

    // toggle light switch state
    // don't need to do this, because scanner should pick up on the changes automatically
    //*lightSwitchOnPtr = *lightSwitchOnPtr ^ 1;

    // start scanner again
    setStatus(start);
}
