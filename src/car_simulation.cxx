#include <iostream>
#include <sstream>

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
		acceleration = 10.f;
		status = IDLE;	
	}
	
	void process_commands(std::string command, car_simulation* car) {
		std::cout << "Command: " << command << std::endl;
	}
	
	void car_simulation::on_message_received(struct mosquitto_message message) {
		if (!message.payload) return;
		
		std::string message_content(static_cast<char*>(message.payload), message.payloadlen);
		size_t lastSlashPos = message_content.find_last_of('/');
		size_t secondLastSlashPos = message_content.find_last_of('/', lastSlashPos - 1);
		
		std::string command = message_content.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1);
		
		process_commands(command, this);
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
		std::string model_file_path = json_helper<std::string>::get_value_or_default("car_model/model_path", "");
		
		if (model_file_path.empty()) {
			std::cerr << "Car model file path not found." << std::endl;
			exit(1);
		}
		
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/model", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_file(model_file_path, setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_acceleration(digital_twin_client &client, float acceleration) 
	{
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/acceleration", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message(std::to_string(acceleration), setting);
		if (rc != MOSQ_ERR_SUCCESS) exit(1);
	}
	
	void publish_gate(digital_twin_client &client, int gate) 
	{
		struct pubsub_setting setting = client.construct_pubsub_setting("registration_topic/topic/acceleration", "registration_topic/qos", "registration_topic/retain");
		
		int rc = client.publish_message(std::to_string(gate), setting);
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
		
		publish_acceleration(client, acceleration);
		
		publish_finish(client);
	}
	
	void turn(float delta_time) {
	
	}
	
	void accelerate(float &velocity, float desired_velocity, float acceleration, float delta_time) {
		float delta_velocity = acceleration * delta_time;
		float new_velocity = velocity + delta_velocity;
		velocity = std::min(new_velocity, desired_velocity);
	}
	
	void move(struct vector_2 &position, struct vector_2 direction, float velocity, float delta_time) {
		float delta_x = direction.x * velocity * delta_time;
		float delta_z = direction.z * velocity * delta_time;
		
		position.x += delta_x;
		position.z += delta_z;
	}
	
	void car_simulation::run(float delta_time) {
		if (status != car_status::RUNNING) return;
			
		if (std::abs(desired_direction.x - direction.x) > 0.001f || std::abs(desired_direction.z - direction.z) > 0.001f) turn(delta_time);
			
		if (std::abs(desired_velocity - velocity) > 0.001f) accelerate(velocity, desired_velocity, acceleration, delta_time);
			
		move(position, direction, velocity, delta_time);
	}
	
}
