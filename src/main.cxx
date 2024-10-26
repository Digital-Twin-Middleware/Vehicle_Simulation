#include <iostream>
#include <chrono>
#include <thread>
#include <string>

#include "digital_twin_client.h"
#include "car_simulation.h"
#include "utility_functions.h"

using namespace digital_twin;

int main() {
    digital_twin_client client;

    client.connect();
	
	car_simulation car(1);
	
	car.register_data(client);
		
	car.run(client);
	
	client.disconnect();
	
    return 0;
}



