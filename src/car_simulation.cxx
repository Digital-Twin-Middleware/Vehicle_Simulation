#include <iostream>

#include "car_simulation.h"
#include "digital_twin_client.h"
#include "json_helper.h"

namespace digital_twin {
	
	int car_simulation::register_car_data(digital_twin_client &client) {
		std::string model_file_path = json_helper<std::string>::get_value_or_default("car_model/model_path", "");
		
		if (model_file_path.empty()) {
			std::cerr << "Car model file path not found." << std::endl;
			exit(1);
		}
		
		struct publish_setting setting = client.construct_publish_setting("registration_topic/topic", "registration_topic/qos", "registration_topic/retain");
		
		return client.publish_file(model_file_path, setting);
	}
}
