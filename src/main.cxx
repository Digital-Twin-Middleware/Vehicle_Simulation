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
	
	car_simulation car;
	
	car.register_data(client);
	
	float delta_time = 0.f;
	
	while (1) {
		delta_time = utility_functions::get_delta_time();
		
		car.run(delta_time);
	}
	
	client.disconnect();
	
    return 0;
}



