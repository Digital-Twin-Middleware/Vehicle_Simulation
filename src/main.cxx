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
	car.register_car_data(client);
	
	client.disconnect();
	
    return 0;
}



