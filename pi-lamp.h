#include "RCSwitch/RCSwitch.h"
#include "clickButton/clickButton.h"
#include <wiringPi.h>

// "addresses" for the relays
#define DILLON_ON   0x451533
#define DILLON_OFF  0x45153C

#define SARA_ON     0x4515C3
#define SARA_OFF    0x4515CC

#define LIQUOR_ON   0x451703
#define LIQUOR_OFF  0x45170C

// length of each bit
#define PULSE_LENGTH 0xBE

// length of packet to send
#define BIT_LENGTH 24

void toggleDillon(RCSwitch mySwitch);
void toggleSara(RCSwitch mySwitch);

void switchLamps(boolean on, RCSwitch mySwitch);

// arguments for matchToggle
typedef enum {
    dillon,
    sara
} LampOwners;

void matchToggle(LampOwners owner, RCSwitch mySwitch);

void toggleLight(void);
