#include <iostream>
#include <thread>

void callFromThread(){
	std::cout << "Hello, World" << std::endl;
}

int main() {
	std::thread t1(callFromThread);

	t1.join();

	return 0;
}
