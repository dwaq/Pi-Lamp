#include "RCSwitch/RCSwitch.h"
#include "clickButton/clickButton.h"
#include <wiringPi.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>

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

// stores the state of the lightSwitch
int lightSwitchOn = 0;
int *lightSwitchOnPtr = &lightSwitchOn;

// to communicate with the thread
int kill = 0;
int *killPtr = &kill;

int scan_status = -1;
int *scan_statusPtr = &scan_status;

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

// http://thispointer.com/c11-multithreading-part-2-joining-and-detaching-threads/
class ThreadRAII
{
    std::thread & m_thread;
    public:
        ThreadRAII(std::thread  & thread) : m_thread(thread)
        {

        }

        ~ThreadRAII()
        {
            // Check if thread is joinable then detach the thread
            if(m_thread.joinable())
            {
                m_thread.detach();
            }
        }
};

void scan_service(void);

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

    // start scanner
    *scan_statusPtr = 1;

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
            // need to kill scanner before connecting to switchmate w/ bluetooth
            *scan_statusPtr = 0;
			*killPtr = 1;

            //printf("TRIPLE click\n");
            toggleLight();

            // start scanner
            *scan_statusPtr = 1;
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
    *scan_statusPtr = 0;
    *killPtr = 1;

    // RESOURCE ACQUISITION IS INITIALIZATION allows us to call detach()
    // in the case of exceptions
    ThreadRAII wrapperObj(thread);

    return 0;
}

void scan_service(){

	// Information on unix domain sockets
	// https://github.com/troydhanson/network/blob/master/unixdomain/srv.c
	// path to our socket
	const char *socket_path = "/tmp/pi-lamp-status";

	// socket stuff
	struct sockaddr_un addr;
	char buf[1], last_buf[1];
	int fd,cl,rc;

	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
	  perror("socket error");
	  exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	if (*socket_path == '\0') {
	  *addr.sun_path = '\0';
	  strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
	} else {
	  strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
	  unlink(socket_path);
	}

	if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
	  perror("bind error");
	  exit(-1);
	}

	if (listen(fd, 5) == -1) {
	  perror("listen error");
	  exit(-1);
	}


	int scanner_running = 0;

	while(1) {



		//std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//std::cout << "Hello, thread. kill: " << *killPtr << std::endl;


		if (scanner_running == 1){

			// check socket
			if ( (cl = accept(fd, NULL, NULL)) == -1) {
			  perror("accept error");
			  continue;
			}

			while ( (rc=read(cl,buf,sizeof(buf))) > 0) {
				//printf("read %u bytes: %.*s\n", rc, rc, buf);

				// if the current buffer is different than the last,
				// change the status
				if (strcmp(buf, last_buf)){
					// convert ASCII character in buffer to integer
					*lightSwitchOnPtr = buf[0] - '0';
					//printf("%.*s %.*s\n", 3, buf, 3, last_buf);
					printf("The lightswitch state is now: %i\n", lightSwitchOn);
				}

				// save current as last
				strcpy (last_buf, buf);
			}
			if (rc == -1) {
			  perror("read");
			  exit(-1);
			}
			else if (rc == 0) {
			  // socket closed
			  //printf("EOF\n");
			  close(cl);
			}
		}

		// start scanner
		if (*scan_statusPtr == 1) {
			std::cout << "start scanner" << std::endl;
			system("/home/pi/Pi-Lamp/Daemon/scan.py start");

			// singnify that the scanner started
			scanner_running = 1;

			// go back to default
			*scan_statusPtr = -1;
		}

		// stop scanner
		if (*scan_statusPtr == 0) {
			std::cout << "stop scanner" << std::endl;
			system("/home/pi/Pi-Lamp/Daemon/scan.py stop");
			// go back to default
			*scan_statusPtr = -1;
		}

		// kill the thread
		if (*killPtr == 1) {
			pthread_exit(NULL);
		}
	}
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
    // if on, turn off
    if (*lightSwitchOnPtr){
        system("./Switchmate/off.sh");
    }
    // if off, turn on
    else {
        system("./Switchmate/on.sh");
    }

    // toggle light switch state
    *lightSwitchOnPtr = *lightSwitchOnPtr ^ 1;
}
