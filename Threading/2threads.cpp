#include <iostream>
#include <thread>

#include <chrono>

int kill = 0;
int *killPtr = &kill;

void callFromThread(){
	while(1) {
		//delay(500);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		std::cout << "Hello, thread. kill: " << *killPtr << std::endl;
		if (*killPtr == 1) {
			pthread_exit(NULL);
		}
	}
}

int main() {
	std::thread t1(callFromThread);

	for (int i = 0; i < 5; ++i) {
		//delay(666);
		std::this_thread::sleep_for(std::chrono::milliseconds(666));

		if (i == 2){
			*killPtr = 1;
		}

		std::cout << "Hello, main. i: " << i << std::endl;
	}

	t1.join();

	return 0;
}
