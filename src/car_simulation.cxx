#include <iostream>
#include <sstream>
#include <cmath>

#include "car_simulation.h"
#include "digital_twin_client.h"
#include "json_helper.h"
#include "utility_functions.h"

namespace digital_twin {
	car_simulation::car_simulation() {
		position = {0.f, 0.f};
		direction = {0.f, 0.f};
		desired_direction = {0.f, 0.f};
		velocity = 0.f;
		desired_velocity = 0.f;
		acceleration = 2.f;
		rotation_speed = 1.f;
		status = IDLE;	
	}
	
	void process_commands(std::string command, std::string message, car_simulation* car) {
		if (command == "velocity") {
			float new_velocity = std::stof(message);
			car->desired_velocity = new_velocity;
		}
		else if (command == "position") {
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
			
			if (command == "set_direction") car->direction = {new_x, new_z};
			
		}
		else if (command == "status") {
			int new_status = std::stoi(message);
			car->status = static_cast<car_status>(new_status); 
		}
		else std::cout << "Command not found: " << command << "." << std::endl;
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
	
	void subscribe(digital_twin_client &client) {
		struct pubsub_setting setting = client.construct_pubsub_setting("command_topic/topic", "command_topic/qos", "");
		
		int rc = client.subscribe(setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_car_model(digital_twin_client &client) {
		std::string model_file_path = json_helper<std::string>::get_value_or_default("car_model/model_1/path", "");
		
		if (model_file_path.empty()) {
			std::cerr << "Car model file path not found." << std::endl;
			exit(1);
		}
		
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/model", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_file(model_file_path, setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_model_rotation_offset(digital_twin_client &client) {
		float rotation_offset = json_helper<float>::get_value_or_default("car_model/model_1/rotation_offset", 0);
		
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/rotation_offset", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message(std::to_string(rotation_offset), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_acceleration(digital_twin_client &client, float acceleration) 
	{
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/acceleration", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message(std::to_string(acceleration), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
		
	int random_gate() {
		int min_gate = json_helper<int>::get_value_or_default("gate/min_gate", 1);
		int max_gate = json_helper<int>::get_value_or_default("gate/max_gate", 1);
		
		return utility_functions::get_random_in_range(min_gate, max_gate);
	}
	
	void publish_gate(digital_twin_client &client) 
	{
		int start_gate = random_gate();
		int end_gate = 0;
		do {
			end_gate = random_gate();
		}
		while (end_gate == start_gate);
		
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/start_gate", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message(std::to_string(start_gate), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
		
		setting = client.construct_pubsub_setting("registration_topic/topic/end_gate", "registration_topic/qos", "registration_topic/retain");
		rc = client.publish_message(std::to_string(end_gate), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_finish(digital_twin_client &client) {
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/ready", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message("1", setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void car_simulation::register_data(digital_twin_client &client) {
        message_received_event = [this](struct mosquitto_message msg) {
            this->on_message_received(msg);
        };
        
        set_callback(client, message_received_event);
		
		subscribe(client);
		
		publish_car_model(client);
		
		publish_model_rotation_offset(client);
		
		publish_acceleration(client, acceleration);
		
		publish_gate(client);
		
		publish_finish(client);
	}
	
	float magnitude(struct vector_2 vector) {
		return sqrt(vector.x * vector.x + vector.z * vector.z);
	}
	
	void rotate(struct vector_2 &direction, float angle) {
		struct vector_2 original_direction = direction;
		
		direction.x = original_direction.x * cos(angle) - original_direction.z * sin(angle);
		direction.z = original_direction.x * sin(angle) + original_direction.z * cos(angle);
	}
	
	void turn(float delta_time, float rotation_speed, struct vector_2 &direction, struct vector_2 desired_direction, digital_twin_client &client) {
		
		float dot_product = direction.x * desired_direction.x + direction.z * desired_direction.z;
		float cross_product = direction.x * desired_direction.z - direction.z * desired_direction.x;
		
		float target_angle = acos(dot_product);
		float rotation_angle = rotation_speed * delta_time;
		
		std::string qos_key;
		
		if (rotation_angle >= target_angle) {
			direction.x = desired_direction.x;
			direction.z = desired_direction.z;
			
			qos_key = "position_topic/qos/important";
		}
		else {
			if (cross_product < 0) rotate(direction, -rotation_angle);
			else rotate(direction, rotation_angle);
			
			qos_key = "position_topic/qos/normal";
		}
		
		struct pubsub_setting setting = client.construct_pubsub_setting("position_topic/topic/direction", qos_key, "position_topic/retain");
		
		std::stringstream ss;
		ss << direction.x << '/' << direction.z;
		std::string message = ss.str();
		
		client.publish_message(message, setting);
	}
	
	void accelerate(float &velocity, float desired_velocity, float acceleration, float delta_time, digital_twin_client &client) {
		float delta_velocity = acceleration * delta_time;
		float new_velocity = velocity + delta_velocity;
		velocity = std::min(new_velocity, desired_velocity);
		
		std::string qos_key = std::abs(desired_velocity - velocity) > 0.001f ? "position_topic/qos/normal" : "position_topic/qos/important";
		struct pubsub_setting setting = client.construct_pubsub_setting("position_topic/topic/velocity", qos_key, "position_topic/retain");
		
		client.publish_message(std::to_string(velocity), setting);
	}
	
	void move(struct vector_2 &position, struct vector_2 direction, float velocity, float delta_time, digital_twin_client &client) {
		float delta_x = direction.x * velocity * delta_time;
		float delta_z = direction.z * velocity * delta_time;
		
		position.x += delta_x;
		position.z += delta_z;
		
		struct pubsub_setting setting = client.construct_pubsub_setting("position_topic/topic/position", "position_topic/qos/normal", "position_topic/retain");
		
		std::stringstream ss;
		ss << position.x << '/' << position.z;
		std::string message = ss.str();
		
		client.publish_message(message, setting);
	}
	
	void car_simulation::run(float delta_time, digital_twin_client &client) {
		if (status != car_status::RUNNING) return;
			
		if (magnitude((struct vector_2){desired_direction.x - direction.x, desired_direction.z - direction.z}) > 0.001f) turn(delta_time, rotation_speed, direction, desired_direction, client);
			
		if (std::abs(desired_velocity - velocity) > 0.001f) accelerate(velocity, desired_velocity, acceleration, delta_time, client);
			
		move(position, direction, velocity, delta_time, client);
	}
	
}
