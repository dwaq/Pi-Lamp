#include "status.h"


/* start scanner when thread is started */
SwitchmateThread status = start;

void setStatus(SwitchmateThread s){
    status = s;
}
SwitchmateThread getStatus(void){
    return status;
}


/* stores the state of the lightSwitch */
int switchState = 0;

void setSwitchState(int state){
    switchState = state;
}
int getSwitchState(void){
    return switchState;
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

	while(1) {
        //
        if (getStatus() == running){

			// check socket
			if ( (cl = accept(fd, NULL, NULL)) == -1) {
			  perror("accept error");
			  continue;
			}

			while ( (rc=read(cl,buf,sizeof(buf))) > 0) {
				// if the current buffer is different than the last,
				// change the status
				if (strcmp(buf, last_buf)){
					// convert ASCII character in buffer to integer
					int state = buf[0] - '0';

                    setSwitchState(state);
					//printf("%.*s %.*s\n", 3, buf, 3, last_buf);
					printf("The lightswitch state is now: %i\n", state);
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
		else if (getStatus() == start) {
			printf("Start status daemon\n");
			system("./Daemon/statusd.py start");

			// go back to running
			status = running;
		}

		// stop scanner
		else if (getStatus() == stop) {
			printf("Stop status daemon");
			system("./Daemon/statusd.py stop");

			// go back to running
			status = running;
		}

		// kill the thread
		else if (getStatus() == kill){
		    // TODO: give up access to the socket file location?
			pthread_exit(NULL);
		}
	}
}
