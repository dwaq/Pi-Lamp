#include <iostream>
#include <thread>

void pr(void){
std::cout << "function called" << std::endl;
}

void callFromThread(){
	std::cout << "Hello, World" << std::endl;
	pr();
}

int main() {
	pr();

	std::thread t1(callFromThread);

	t1.join();

	return 0;
}
