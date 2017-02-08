#include <iostream>
#include <thread>

static const int num_threads = 10;

void callFromThread(int tid){
	std::cout << "Hello, " << tid << std::endl;
}

int main() {
	std::thread t[num_threads];

	for (int i = 0; i < num_threads; ++i) {
		t[i] = std::thread(callFromThread, i);
	}

	std::cout << "main thread\n";

	for (int i=0; i<num_threads; ++i) {
		t[1].join();
	}

	return 0;
}
