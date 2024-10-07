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
	
	void init_mqtt_client(struct mosquitto **mqtt_client) {
		std::string client_id = json_helper<std::string>::get_value_or_default("connection/client_id", "Undefined");
		bool clean_session = json_helper<bool>::get_value_or_default("connection/clean_session", true);
		
		std::stringstream ss;
		ss << client_id << '_' << getpid();
		client_id = ss.str();
		
		*mqtt_client = mosquitto_new(client_id.c_str(), clean_session, nullptr);
		
		if (*mqtt_client) return;
		
		std::cerr << "Failed to create mosquitto instance." << std::endl;
		exit(1);
 	}
 	
 	void init_will(struct mosquitto *mqtt_client) {
 		//TODO Later
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
	
	digital_twin_client::digital_twin_client() {
		mqtt_client = nullptr;
		
		get_mosquitto_mqtt_version();
		
		init_mosquitto_lib();
		
		init_mqtt_client(&mqtt_client);
		
		init_user(mqtt_client);
		
		init_publishing_setting(file_publishing_chunk_size);
	}
	
	void clean_up(struct mosquitto *mqtt_client) {
		mosquitto_loop_stop(mqtt_client, false);
		
		if (mqtt_client) mosquitto_destroy(mqtt_client);
    	
    	mosquitto_lib_cleanup();
	}
	
	digital_twin_client::~digital_twin_client() {
    	clean_up(mqtt_client);
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
    
     struct publish_setting digital_twin_client::construct_publish_setting(std::string topic_key, std::string qos_key, std::string retain_key) {
 		std::string publish_topic = json_helper<std::string>::get_value_or_default(topic_key, "");
 		std::stringstream ss;
		ss << publish_topic << '/' << getpid();
		publish_topic = ss.str();
    	
    	int publish_qos_level = json_helper<int>::get_value_or_default(qos_key, 0);
    	bool publish_message_retain = json_helper<bool>::get_value_or_default(retain_key, false);
    	
    	struct publish_setting setting = {publish_topic, publish_qos_level, publish_message_retain};
    	
    	return setting;
 	}
    
    void digital_twin_client::disconnect() {
    	int rc = mosquitto_disconnect(mqtt_client);
    	if (rc == MOSQ_ERR_SUCCESS) {
    		clean_up(mqtt_client);
    		std::cout << "Successfully disconnected from the broker." << std::endl;
    	} else {
    		std::cout << "Fail to disconnect from the broker: " << mosquitto_strerror(rc) << std::endl;
    	}
    }
    
    int digital_twin_client::publish_message(std::string message, struct publish_setting setting) {
    	const char* new_message = message.c_str();
    	
    	int rc = mosquitto_publish(mqtt_client, nullptr, setting.topic.c_str(), strlen(new_message), new_message, setting.qos_level, setting.retain);
    	if (rc != MOSQ_ERR_SUCCESS) std::cout << "Fail to publish messages: " << mosquitto_strerror(rc) << std::endl;
    	
    	return rc;
    }
    
    int digital_twin_client::publish_file(std::string file_path, struct publish_setting setting) {
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
}


