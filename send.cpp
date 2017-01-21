/*
 Usage: ./send <command>
 Command is 0 for OFF and 1 for ON
 */

#include "RCSwitch.h"
#include <stdlib.h>
#include <stdio.h>

// "addresses" for the relays
#define LIQUOR_ON   0x451703
#define LIQUOR_OFF  0x45170C

// length of each bit
#define PULSE_LENGTH 190     // 0xBE

// length of packet to send
#define BIT_LENGTH 24

int main(int argc, char *argv[]) {

    // pin 3 is really GPIO 22 on the Pi
    int PIN = 3;            // 433 Mhz transmitter
    int command  = atoi(argv[1]);

    if (wiringPiSetup () == -1) return 1;
	printf("sending command[%i]\n", command);
	RCSwitch mySwitch = RCSwitch();
    // setup the transmitter on pin 3
	mySwitch.enableTransmit(PIN);

    // Set pulse length of a bit
    mySwitch.setPulseLength(PULSE_LENGTH);

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
