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

    // don't allow another long press to occur until the last one has been dealt with
    int blockDillonLongPress = 0;
    int blockSaraLongPress = 0;

    while(1) {
        // print state change
        newState = getSwitchState();
        if (oldState != newState) {
            std::cout << "Switchmate state changed to: " << newState << std::endl;
            // print all bytes of Switchmate data
            std::cerr << "Switchmate data is: ";
            printSwitchmateData();

            // skip setting lamps
            if (getSkipSwitchMatch()) {
              // reset variable for next time
              setSkipSwitchMatch(0);
            }
            // otherwise, switched via switchmate, so change lamps to match
            else {
              // switch lamps to match Switchmate
              if (newState == 1){
                bothOn();
              }
              else {
                bothOff();
              }
            }
        }
        oldState = newState;

        // Update Dillon's button state
        dillonButton.Update();

        // Dillon's button was clicked
        if (dillonButton.clicks != 0) dillonClicks = dillonButton.clicks;

        if(dillonClicks == 1){
            std::cout << "SINGLE clicked Dillon's button." << std::endl;
            toggleDillon();
        }

        if(dillonClicks == 2){
            std::cout << "DOUBLE clicked Dillon's button." << std::endl;
            toggleSara();
        }

        if(dillonClicks == 3){
            std::cout << "TRIPLE clicked Dillon's button." << std::endl;
            toggleLight();
        }

        if((dillonClicks == -1) && (blockDillonLongPress != -1)){
            std::cout << "SINGLE LONG clicked Dillon's button." << std::endl;
            // alert user that button has been held for long enough
            switchLamp(dillon, alert);
            // then toggle all
            matchToggle(dillon);
        }

        // block long press from occuring again until it's changed to something else
        blockDillonLongPress = dillonClicks;

        // reset counter for next round
        dillonClicks = 0;

        // update Sara's button state
        saraButton.Update();

        // Sara's button was clicked
        if (saraButton.clicks != 0) saraClicks = saraButton.clicks;

        if(saraClicks == 1){
            std::cout << "SINGLE clicked Sara's button." << std::endl;
            toggleSara();
        }

        if(saraClicks == 2){
            std::cout << "DOUBLE clicked Sara's button." << std::endl;
            toggleDillon();
        }

        if(saraClicks == 3){
            std::cout << "TRIPLE clicked Sara's button." << std::endl;
            toggleLight();
        }

        if((saraClicks == -1) && (blockSaraLongPress == 0)){
            std::cout << "SINGLE LONG clicked Sara's button." << std::endl;
            // alert user that button has been held for long enough
            switchLamp(sara, alert);
            // then toggle all
            matchToggle(sara);
        }

        // block long press from occuring again until it's changed to something else
        blockSaraLongPress = saraClicks;

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
void matchToggle(LampOwners owner){
    // if owner's light is currently on,
    if (lampStatus(owner) == 1){
        // turn both off
        switchLamp(both, off);

        // turn switchmate off if not already off
        if (getSwitchState() != 0){
            toggleLight();
        }
    }
    // else, owner's light is off,
    else {
        // turn both on
        switchLamp(both, on);

        // turn switchmate on if not already on
        if (getSwitchState() != 1){
            toggleLight();
        }
    }
}

/* toggles the overhead light using a Switchmate */
void toggleLight(void){
    // need to kill scanner before connecting to Switchmate w/ bluetooth
    std::cout << "Cancelling Bluetooth scan" << std::endl;
    cancelScan();
    // wait for thread to terminate
    std::cout << "Waiting for thread to terminate" << std::endl;
    if(thread.joinable())
    {
        std::cout << "Joining thread" << std::endl;
        thread.join();
    }

    // if on, turn off
    if (getSwitchState()){
        std::cout << "Turning off Switchmate" << std::endl;
        system("./switchmate/off.sh");
    }
    // if off, turn on
    else {
        std::cout << "Turning on Switchmate" << std::endl;
        system("./switchmate/on.sh");
    }

    // start scanner again
    std::cout << "Restarting scanner" << std::endl;
    thread = std::thread(scan_service);

    // set skip state so lamps don't automatically toggle
    setSkipSwitchMatch(1);

    std::cout << "toggleLight() completed" << std::endl;
}
