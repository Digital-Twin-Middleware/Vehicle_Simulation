#include <iostream>

#include "car_simulation.h"
#include "digital_twin_client.h"
#include "json_helper.h"

namespace digital_twin {
	car_simulation::car_simulation() {
		position = {0.f, 0.f};
		direction = {0.f, 0.f};
		desired_direction = {0.f, 0.f};
		velocity = 0.f;
		desired_velocity = 0.f;
		has_started = false;
		has_finished = false;	
	}
	
	void car_simulation::register_data(digital_twin_client &client) {
		std::string model_file_path = json_helper<std::string>::get_value_or_default("car_model/model_path", "");
		
		if (model_file_path.empty()) {
			std::cerr << "Car model file path not found." << std::endl;
			exit(1);
		}
		
		struct publish_setting setting = client.construct_publish_setting("registration_topic/topic", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_file(model_file_path, setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void turn(float delta_time) {
	
	}
	
	void accelerate(float delta_time) {
	
	}
	
	void move(float delta_time) {
	
	}
	
	void car_simulation::run(float delta_time) {
		if (has_finished || !has_started) return;
			
		if (std::abs(desired_direction.x - direction.x) > 0.001f || std::abs(desired_direction.z - direction.z) > 0.001f) turn(delta_time);
			
		if (std::abs(desired_velocity - velocity) > 0.001f) accelerate(delta_time);
			
		move(delta_time);
	}
}
