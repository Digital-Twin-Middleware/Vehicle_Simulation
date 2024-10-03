#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include "digital_twin_client.h"

using namespace digital_twin;

int main() {
    digital_twin_client client;

    client.connect();
	
	for (int i = 0; i < 10; i++) {
		client.publish("Hello, World!");
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	
	client.disconnect();
	
    return 0;
}



