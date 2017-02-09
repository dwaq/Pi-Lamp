#include <iostream>
#include <thread>

#include <chrono>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>


int kill = 0;
int *killPtr = &kill;

int data = 0;
int *dataPtr = &data;

int scan_status = -1;
int *scan_statusPtr = &scan_status;


// stores the state of the lightSwitch
int lightSwitchOn = 0;
int *lightSwitchOnPtr = &lightSwitchOn;


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

		// read socket

		if (*killPtr == 1) {
			*dataPtr = 1;
			pthread_exit(NULL);
		}
	}
}

int main() {
	std::thread thread(scan_service);

	for (int i = 0; i < 5; ++i) {
		//delay(666);
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));

		if (i == 1){
			*scan_statusPtr = 1;
		}

		if (i == 3){
			*scan_statusPtr = 0;
			*killPtr = 1;
		}

		//std::cout << "data: " << *dataPtr << std::endl;
	}

	thread.join();

	return 0;
}
