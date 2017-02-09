#include <iostream>
#include <thread>

#include <chrono>

#include <sys/socket.h>

int kill = 0;
int *killPtr = &kill;

int data = 0;
int *dataPtr = &data;

int scan_status = -1;
int *scan_statusPtr = &scan_status;

void scan_service(){
	while(1) {
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));
		//std::cout << "Hello, thread. kill: " << *killPtr << std::endl;

		// start scanner
		if (*scan_statusPtr == 1) {
			system("/home/pi/Pi-Lamp/Daemon/scan.py start");
			// go back to default
			*scan_statusPtr = -1;
		}

		// stop scanner
		if (*scan_statusPtr == 0) {
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
		std::this_thread::sleep_for(std::chrono::milliseconds(666));

		if (i == 1){
			*scan_statusPtr = 1;
		}

		if (i == 3){
			*scan_statusPtr = 0;
			*killPtr = 1;
		}

		std::cout << "data: " << *dataPtr << std::endl;
	}

	thread.join();

	return 0;
}
