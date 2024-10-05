#include <iostream>

#include "car_simulation.h"
#include "digital_twin_client.h"
#include "json_helper.h"

namespace digital_twin {
	
	void car_simulation::register_car_data(digital_twin_client client) {
		std::string model_file_path = json_helper<std::string>::get_value_or_default("car_model/model_path", "");
	
		struct publish_setting setting = client.construct_publish_setting("registration_topic/topic", "registration_topic/qos", "registration_topic/retain");
	}
}
