#include "pi-lamp.h"

// create global thread for status
// global required because killing from a function
std::thread thread(scan_service);

int main(void) {
    int dillonLamp = 23;    // Dillon's lamp switch
    int saraLamp = 21;      // Sara's lamp switch

    if (wiringPiSetup () == -1) return 1;

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

    // get light switch status at startup
    int oldState = getSwitchState();
    int newState = oldState;

    while(1) {
        // print state change
        newState = getSwitchState();
        if (oldState != newState) {
            printf("Switchmate state changed to: %i\n", newState);

            // switch lamps to match Switchmate
            if (newState == 1){
                switchBoth(true);
            }
            else {
                switchBoth(false);
            }
        }
        oldState = newState;

        // Update Dillon's button state
        dillonButton.Update();

        // Dillon's button was clicked
        if (dillonButton.clicks != 0) dillonClicks = dillonButton.clicks;

        if(dillonClicks == 1){
            //printf("SINGLE click\n");
            toggleDillon();
        }

        if(dillonClicks == 2){
            //printf("DOUBLE click\n");
            toggleSara();
        }

        if(dillonClicks == 3){
            //printf("TRIPLE click\n");
            toggleLight();
        }
        /*
        if(dillonClicks == -1){
            //printf("SINGLE LONG click\n");
            matchToggle(dillon);
        }
        */
        // reset counter for next round
        dillonClicks = 0;

        // update Sara's button state
        saraButton.Update();

        // Sara's button was clicked
        if (saraButton.clicks != 0) saraClicks = saraButton.clicks;

        if(saraClicks == 1){
            //printf("SINGLE click\n");
            toggleSara();
        }

        if(saraClicks == 2){
            //printf("DOUBLE click\n");
            toggleDillon();
        }

        if(saraClicks == 3){
            //printf("TRIPLE click\n");
            toggleLight();
        }
        /*
        if(saraClicks == -1){
            //printf("SINGLE LONG click\n");
            matchToggle(sara);
        }
        */
        // reset counter for next round
        saraClicks = 0;

        delay(5);
    }

    // RESOURCE ACQUISITION IS INITIALIZATION allows us to call detach()
    // in the case of exceptions
    ThreadRAII wrapperObj(thread);

    return 0;
}




/* set both lamps to the opposite of the button's lamp's current state */
/*
void matchToggle(LampOwners owner){
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
        switchLamps(false);

        // toggle switchmate if not off
        if (getSwitchState() != 0){
            toggleLight();
        }
    }
    // currently off, so turn both on
    else {
        switchLamps(true);

        // toggle switchmate if not on
        if (getSwitchState() != 1){
            toggleLight();
        }
    }
}
*/

/* toggles the overhead light using a Switchmate */
void toggleLight(void){
    // need to kill scanner before connecting to Switchmate w/ bluetooth
    cancelScan();
    // wait for thread to terminate
    if(thread.joinable())
    {
        thread.join();
    }

    // if on, turn off
    if (getSwitchState()){
        system("./switchmate/off.sh");
    }
    // if off, turn on
    else {
        system("./switchmate/on.sh");
    }

    // start scanner again
    thread = std::thread(scan_service);
}
