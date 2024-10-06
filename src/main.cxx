#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include "digital_twin_client.h"
#include "car_simulation.h"

using namespace digital_twin;

int main() {
    digital_twin_client client;

    client.connect();
	
	car_simulation car;
	
	int rc = car.register_car_data(client);
	
	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	client.disconnect();
	
    return 0;
}



