#include <string>
#include <iostream>
#include <mosquitto.h>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <fstream>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
#include <algorithm>

#include "digital_twin_client.h"
#include "json_helper.h"

using json = nlohmann::json;

namespace digital_twin {	
	void get_mosquitto_mqtt_version() {
		int major = 0;
		int minor = 0;
		int revision = 0;
		
		mosquitto_lib_version(&major, &minor, &revision);
		
		std::cout << "Using Mosquitto MQTT version: " << major << "." << minor << "." << revision << "." << std::endl;
	}
	
	void init_mosquitto_lib() {
		int rc = mosquitto_lib_init();
		if (rc == MOSQ_ERR_SUCCESS) return;
		
		std::cerr << "Initialize mosquitto library error: " << mosquitto_strerror(rc) << std::endl;
		exit(1);
	}
	
	void init_mqtt_client(struct mosquitto **mqtt_client, digital_twin_client *client) {
		std::string client_id = json_helper<std::string>::get_value_or_default("connection/client_id", "Undefined");
		bool clean_session = json_helper<bool>::get_value_or_default("connection/clean_session", true);
		
		std::stringstream ss;
		ss << client_id << '_' << gettid();
		client_id = ss.str();
		
		*mqtt_client = mosquitto_new(client_id.c_str(), clean_session, static_cast<void*>(client));
		
		if (*mqtt_client) return;
		
		std::cerr << "Failed to create mosquitto instance." << std::endl;
		exit(1);
 	}
 	
 	void init_will(struct mosquitto *mqtt_client, int id) {
 		std::string last_will_topic = json_helper<std::string>::get_value_or_default("last_will_topic/topic", "");
 		bool last_will_retain = json_helper<bool>::get_value_or_default("last_will_topic/retain", false);
 		int last_will_qos = json_helper<int>::get_value_or_default("last_will_topic/qos", 0);
 		
 		const char* last_will_message = std::to_string(id).c_str();
 		
 		mosquitto_will_set(mqtt_client, last_will_topic.c_str(), strlen(last_will_message), last_will_message, last_will_qos, last_will_retain);
 	}
 	
 	void init_user(struct mosquitto *mqtt_client) {
 		std::string username = json_helper<std::string>::get_value_or_default("connection/username", "");
 		std::string password = json_helper<std::string>::get_value_or_default("connection/password", "");
 		
 		int rc = mosquitto_username_pw_set(mqtt_client, username.c_str(), password.c_str());
 		
 		if (rc == MOSQ_ERR_SUCCESS) return;
 		
 		std::cerr << "Initialize mosquitto user error: " << mosquitto_strerror(rc) << std::endl;
		exit(1);
 	}
 	
 	void init_publishing_setting(int &file_publishing_chunk_size) {
 		int chunk_size = json_helper<int>::get_value_or_default("publish_setting/file_publishing_chunk_size", 1024);
 		
 		file_publishing_chunk_size = chunk_size;
 	}
 	
 	void on_message_received(struct mosquitto *mqtt_client, void* user_data, const struct mosquitto_message *message) {
 		auto client = static_cast<digital_twin_client*>(user_data);
 		
 		client->invoke_message_received_event(*message);
 	}
	
	digital_twin_client::digital_twin_client(int id) {
		mqtt_client = nullptr;
		
		get_mosquitto_mqtt_version();
		
		init_mosquitto_lib();
		
		init_mqtt_client(&mqtt_client, this);
		
		init_user(mqtt_client);
		
		init_publishing_setting(file_publishing_chunk_size);
		
		init_will(mqtt_client, id);
		
		message_received_callbacks.clear();
		
		mosquitto_message_callback_set(mqtt_client, on_message_received);
	}
	
	void clean_up(struct mosquitto *mqtt_client) {
		mosquitto_loop_stop(mqtt_client, false);
		
		if (mqtt_client) mosquitto_destroy(mqtt_client);
    	
    	mosquitto_lib_cleanup();
	}

    void digital_twin_client::connect() {
		std::string broker_address = json_helper<std::string>::get_value_or_default("connection/broker_address", "");
		int port = json_helper<int>::get_value_or_default("connection/port", 1883);
		int alive_time = json_helper<int>::get_value_or_default("connection/alive_time", 10);
		
		int rc = mosquitto_connect(mqtt_client, broker_address.c_str(), port, alive_time);
		if (rc == MOSQ_ERR_SUCCESS) std::cout << "Successfully connected to the broker." << std::endl;
		else {
			std::cout << "Fail to connect to the broker: " << mosquitto_strerror(rc) << std::endl;
			clean_up(mqtt_client);
			exit(1);
		}
		
		std::cout << "Establishing network traffic ..." << std::endl;
		rc = mosquitto_loop_start(mqtt_client);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		if (rc == MOSQ_ERR_SUCCESS) std::cout << "Successfully established the network traffic." << std::endl;
		else {
			std::cout << "Fail to established the network traffic: " << mosquitto_strerror(rc) << std::endl;
			clean_up(mqtt_client);
			exit(1);
		}
    }
    
     struct pubsub_setting digital_twin_client::construct_pubsub_setting(int id, std::string topic_key, std::string qos_key, std::string retain_key) {
 		std::string publish_topic = json_helper<std::string>::get_value_or_default(topic_key, "");
 		
 		std::stringstream ss;
		ss << publish_topic << '/' << id;
		publish_topic = ss.str();
    	
    	int publish_qos_level = json_helper<int>::get_value_or_default(qos_key, 0);
    	bool publish_message_retain = (!retain_key.empty()) ? json_helper<bool>::get_value_or_default(retain_key, false) : false;
    	
    	struct pubsub_setting setting = {publish_topic, publish_qos_level, publish_message_retain};
    	
    	return setting;
 	}
    
    void digital_twin_client::disconnect() {
    	std::cout << "Disconnect trigger." << std::endl;
    	int rc = mosquitto_disconnect(mqtt_client);
    	if (rc == MOSQ_ERR_SUCCESS) {
    		clean_up(mqtt_client);
    		std::cout << "Successfully disconnected from the broker." << std::endl;
    	} else {
    		std::cout << "Fail to disconnect from the broker: " << mosquitto_strerror(rc) << std::endl;
    	}
    }
    
    int digital_twin_client::publish_message(std::string message, struct pubsub_setting setting) {	
    	const char* new_message = message.c_str();
    	
    	int rc = mosquitto_publish(mqtt_client, nullptr, setting.topic.c_str(), strlen(new_message), new_message, setting.qos_level, setting.retain);
    	if (rc != MOSQ_ERR_SUCCESS) std::cout << "Fail to publish messages: " << mosquitto_strerror(rc) << std::endl;
    	
    	return rc;
    }
    
    int digital_twin_client::publish_file(std::string file_path, struct pubsub_setting setting) {
    	std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    	if (!file) {
        	std::cerr << "Failed to open file: " << file_path << "." << std::endl;
        	return -1;
    	}
    
    	std::streamsize file_size = file.tellg();
    	file.seekg(0, std::ios::beg);
		
		char buffer[file_publishing_chunk_size];
    	while (file.read(buffer, file_publishing_chunk_size) || file.gcount() > 0) {
        	std::string payload(buffer, file.gcount());
        	int rc = mosquitto_publish(mqtt_client, nullptr, setting.topic.c_str(), payload.size(), payload.c_str(), setting.qos_level, setting.retain);
        	if (rc != MOSQ_ERR_SUCCESS) {
        		std::cerr << "Fail to send file chunks: " <<  mosquitto_strerror(rc) << std::endl;
        		return rc;
        	}
    	}

    	std::string eof_message = "EOF";
    	int rc = mosquitto_publish(mqtt_client, nullptr, setting.topic.c_str(), eof_message.size(), eof_message.c_str(), setting.qos_level, setting.retain);
		if (rc != MOSQ_ERR_SUCCESS) std::cerr << "Fail to send EOF message: " << mosquitto_strerror(rc) << std::endl;
    	else std::cout << "File transfer complete!" << std::endl;
    	
    	return MOSQ_ERR_SUCCESS;
    }
    
    int digital_twin_client::subscribe(struct pubsub_setting setting) {
    	int rc = mosquitto_subscribe(mqtt_client, nullptr, setting.topic.c_str(), setting.qos_level);
    	if (rc != MOSQ_ERR_SUCCESS) std::cout << "Fail to subscribe to topic: " << mosquitto_strerror(rc) << std::endl;
    	
    	return rc;
    }
    
    void digital_twin_client::register_message_received_callback(std::function<void(struct mosquitto_message)> callback) {
    	message_received_callbacks.push_back(callback);
    }
    
    void digital_twin_client::unregister_message_received_callback(std::function<void(struct mosquitto_message)> callback) {
    	message_received_callbacks.erase(std::remove_if(message_received_callbacks.begin(), message_received_callbacks.end(),
            [&](const std::function<void(struct mosquitto_message)> &registeredCallback) {
                return registeredCallback.target<void(std::string)>() == callback.target<void(std::string)>();
            }), message_received_callbacks.end());
    }
    
    void digital_twin_client::invoke_message_received_event(struct mosquitto_message message) {
    	for (auto &callback : message_received_callbacks) {
 			callback(message);
 		}
    }
}


