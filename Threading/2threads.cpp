#include <iostream>
#include <thread>

#include <chrono>

void callFromThread(){
	while(1) {
		//delay(500);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		std::cout << "Hello, thread" << std::endl;
	}
}

int main() {
	std::thread t1(callFromThread);

	while(1) {
		//delay(666);
		std::this_thread::sleep_for(std::chrono::milliseconds(666));
		std::cout << "Hello, main" << std::endl;
	}

	t1.join();

	return 0;
}
