#include <iostream>
#include <sstream>
#include <cmath>
#include <atomic>

#include "car_simulation.h"
#include "digital_twin_client.h"
#include "json_helper.h"
#include "utility_functions.h"

namespace digital_twin {
	void set_moving_straight_velocity(float &velocity) {
		velocity = json_helper<float>::get_value_or_default("velocity/straight", 0.f);
	}
	
	std::string get_car_key(int car_model_id, std::string suffix) {
		std::stringstream ss;
		ss << "car_model/model_" << car_model_id << suffix;
		std::string car_model_key = ss.str();
		
		return car_model_key;
	}
	
	car_simulation::car_simulation(int car_id) {
		this->car_id = car_id;
		car_model_id = 1;
		position = {0.f, 0.f};
		direction = {0.f, 0.f};
		desired_direction = {0.f, 0.f};
		next_intersection = {0.f, 0.f};
		rotation_speed = json_helper<float>::get_value_or_default("rotation_speed", 0.f);
		set_moving_straight_velocity(velocity);
		
		std::string car_key = get_car_key(car_model_id, "/car_front_offset");
		car_front_offset = json_helper<float>::get_value_or_default(car_key, 0.f);
		
		is_turning = false;
		status = IDLE;	
	}
	
	float magnitude_vector(struct vector_2 vector) {
		return vector.x * vector.x + vector.z * vector.z;
	}
	
	float angle_between_two_normalized_vectors(struct vector_2 first_vector, struct vector_2 second_vector) {
		float dot_product = first_vector.x * second_vector.x + first_vector.z * second_vector.z;
		
		return acos(dot_product);
	}
	
	void process_commands(std::string command, std::string message, car_simulation* car) {
		if (command == "position") {
		
			size_t slashPos = message.find('/');
				
			float new_x = std::stof(message.substr(0, slashPos));
			float new_z = std::stof(message.substr(slashPos + 1));

			car->position = {new_x, new_z};
		}
		else if (command == "direction" || command == "set_direction") {
			
			size_t slashPos = message.find('/');
				
			float new_x = std::stof(message.substr(0, slashPos));
			float new_z = std::stof(message.substr(slashPos + 1));
			
			car->desired_direction = {new_x, new_z};
			
			if (command == "set_direction") {
				car->direction = {new_x, new_z};
			}
			else {
				if (magnitude_vector((struct vector_2){car->direction.x - new_x, car->direction.z - new_z}) < 0.01f) {
					set_moving_straight_velocity(car->velocity);
				}
				else {
					car->is_turning = true;
					float cross_product = car->direction.x * car->desired_direction.z - car->direction.z * car->desired_direction.x;
					car->velocity = cross_product < 0 ? json_helper<float>::get_value_or_default("velocity/right", 0.f) : json_helper<float>::get_value_or_default("velocity/left", 0.f);
				}
			}
		}
		else if (command == "intersection") {
			size_t slashPos = message.find('/');
			
			float new_x = std::stof(message.substr(0, slashPos));
			float new_z = std::stof(message.substr(slashPos + 1));
			
			car->next_intersection = {new_x, new_z};
		}
		else if (command == "status") {
			int new_status = std::stoi(message);
			car->status = static_cast<car_status>(new_status); 
		}
		else std::cerr << "Command not found: " << command << "." << std::endl;
	}
	
	void car_simulation::on_message_received(struct mosquitto_message message) {
		if (!(message.payload && message.topic)) return;
		
		std::string topic = message.topic;

		size_t lastSlashPos = topic.find_last_of('/');
		size_t secondLastSlashPos = topic.find_last_of('/', lastSlashPos - 1);
		
		std::string command = topic.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1);
		
		std::string payload(static_cast<char*>(message.payload), message.payloadlen);
		
		process_commands(command, payload, this);
	}
	
	void set_callback(digital_twin_client &client, std::function<void(struct mosquitto_message)> &message_received_event) {
		client.register_message_received_callback(message_received_event);
	}
	
	void subscribe(int car_id, digital_twin_client &client) {
		struct pubsub_setting setting = client.construct_pubsub_setting(car_id, "command_topic/topic", "command_topic/qos", "");
		
		int rc = client.subscribe(setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_car_model(int car_id, int car_model_id, digital_twin_client &client) {
		std::string car_model_key = get_car_key(car_model_id, "/path");
		std::string model_file_path = json_helper<std::string>::get_value_or_default(car_model_key, "");
		
		if (model_file_path.empty()) {
			std::cerr << "Car model file path not found." << std::endl;
			exit(1);
		}
		
		struct pubsub_setting setting = client.construct_pubsub_setting(car_id, "registration_topic/topic/model", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_file(model_file_path, setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_model_rotation_offset(int car_id, int car_model_id, digital_twin_client &client) {
		std::string car_rotation_offset_key = get_car_key(car_model_id, "/rotation_offset");
		float rotation_offset = json_helper<float>::get_value_or_default(car_rotation_offset_key, 0);
		
		struct pubsub_setting setting = client.construct_pubsub_setting(car_id, "registration_topic/topic/rotation_offset", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message(std::to_string(rotation_offset), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
		
	int random_gate() {
		int min_gate = json_helper<int>::get_value_or_default("gate/min_gate", 1);
		int max_gate = json_helper<int>::get_value_or_default("gate/max_gate", 1);
		
		return utility_functions::get_random_in_range(min_gate, max_gate);
	}
	
	void publish_gate(int car_id, digital_twin_client &client) 
	{
		int start_gate = random_gate();
		int end_gate = 0;
		do {
			end_gate = random_gate();
		}
		while (end_gate == start_gate);
		
		struct pubsub_setting setting = client.construct_pubsub_setting(car_id, "registration_topic/topic/start_gate", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message(std::to_string(start_gate), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
		
		setting = client.construct_pubsub_setting(car_id, "registration_topic/topic/end_gate", "registration_topic/qos", "registration_topic/retain");
		rc = client.publish_message(std::to_string(end_gate), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_finish(int car_id, digital_twin_client &client) {
		struct pubsub_setting setting = client.construct_pubsub_setting(car_id, "registration_topic/topic/ready", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message("1", setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void car_simulation::register_data(digital_twin_client &client) {
        message_received_event = [this](struct mosquitto_message msg) {
            this->on_message_received(msg);
        };
        
        set_callback(client, message_received_event);
		
		subscribe(car_id, client);
		
		publish_car_model(car_id, car_model_id, client);
		
		publish_model_rotation_offset(car_id, car_model_id, client);
		
		publish_gate(car_id, client);
		
		publish_finish(car_id, client);
	}
	
	void rotate(struct vector_2 &direction, float angle) {
		struct vector_2 original_direction = direction;
		
		direction.x = original_direction.x * cos(angle) - original_direction.z * sin(angle);
		direction.z = original_direction.x * sin(angle) + original_direction.z * cos(angle);
	}
	
	
	void turn(int car_id, int car_model_id, float delta_time, float &velocity, float rotation_speed, struct vector_2 &direction, struct vector_2 desired_direction, bool &is_turning, digital_twin_client &client, bool is_pass_frame) {
		
		float target_angle = angle_between_two_normalized_vectors(direction, desired_direction);
		float cross_product = direction.x * desired_direction.z - direction.z * desired_direction.x;
		
		float rotation_angle = rotation_speed * delta_time;
		
		std::string qos_key;
		
		if (rotation_angle >= target_angle) {
			direction.x = desired_direction.x;
			direction.z = desired_direction.z;
			
			is_turning = false;
			set_moving_straight_velocity(velocity);
			qos_key = "position_topic/qos/important";
		}
		else {
			if (cross_product < 0) rotate(direction, -rotation_angle);
			else rotate(direction, rotation_angle);
			
			qos_key = "position_topic/qos/normal";
		}
		
		struct pubsub_setting setting = client.construct_pubsub_setting(car_id, "position_topic/topic/direction", qos_key, "position_topic/retain");
		
		std::stringstream ss;
		ss << direction.x << '/' << direction.z;
		std::string message = ss.str();
		
		if (setting.qos_level > 0) client.publish_message(message, setting);
		else {
			if (is_pass_frame) client.publish_message(message, setting);
		}
	}
	
	void move(int car_id, car_status status, struct vector_2 &position, struct vector_2 direction, float velocity, float delta_time, digital_twin_client &client, bool is_pass_frame) {
		float delta_x = direction.x * velocity * delta_time;
		float delta_z = direction.z * velocity * delta_time;
		
		position.x += delta_x;
		position.z += delta_z;
		
		struct pubsub_setting setting = client.construct_pubsub_setting(car_id, "position_topic/topic/position", "position_topic/qos/normal", "position_topic/retain");
		
		std::stringstream ss;
		ss << position.x << '/' << position.z;
		std::string message = ss.str();
		
		if (is_pass_frame || status == car_status::WAITING) client.publish_message(message, setting);
	}
	
	void check_intersection(float car_front_offset, struct vector_2 position, struct vector_2 direction, struct vector_2 next_intersection, car_status &status) {
		
		bool is_collide = std::abs((next_intersection.x - (position.x + car_front_offset * direction.x))) < 0.5f && std::abs((next_intersection.z - (position.z + car_front_offset * direction.z))) < 0.5f;
		
		if (is_collide && status != car_status::BLOCKING) status = car_status::WAITING;
	}
	
	void car_simulation::run(digital_twin_client &client,  std::atomic<bool>& running) {
		
		utility_functions delta_time_manager;
		
		while (status != car_status::FINISH && running.load()) {
		
			float delta_time = delta_time_manager.get_delta_time();
		
			if (status != car_status::RUNNING) continue;
			
			bool is_pass_frame = delta_time_manager.is_pass_frame();
			
			if (is_turning) turn(car_id, car_model_id, delta_time, velocity, rotation_speed, direction, desired_direction, is_turning, client, is_pass_frame);
		
			check_intersection(car_front_offset, position, direction, next_intersection, status);
		
			move(car_id, status, position, direction, velocity, delta_time, client, is_pass_frame);
		}
	}
	
}
