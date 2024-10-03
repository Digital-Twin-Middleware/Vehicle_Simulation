#include <string>
#include <iostream>
#include <mosquitto.h>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sstream>

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
		std::string client_id_prefix = json_helper<std::string>::get_value_or_default("client_id", "Undefined_");
		bool clean_session = json_helper<bool>::get_value_or_default("clean_session", true);
		
		std::stringstream ss;
		ss << client_id_prefix << getpid();
		std::string client_id = ss.str();
		
		*mqtt_client = mosquitto_new(client_id.c_str(), clean_session, nullptr);
		
		if (*mqtt_client) return;
		
		std::cerr << "Failed to create mosquitto instance." << std::endl;
		exit(1);
 	}
 	
 	void init_will(struct mosquitto *mqtt_client) {
 		//TODO Later
 	}
 	
 	void init_user(struct mosquitto *mqtt_client) {
 		std::string username = json_helper<std::string>::get_value_or_default("username", "");
 		std::string password = json_helper<std::string>::get_value_or_default("password", "");
 		
 		int rc = mosquitto_username_pw_set(mqtt_client, username.c_str(), password.c_str());
 		
 		if (rc == MOSQ_ERR_SUCCESS) return;
 		
 		std::cerr << "Initialize mosquitto user error: " << mosquitto_strerror(rc) << std::endl;
		exit(1);
 	}
	
	void clean_up(struct mosquitto *mqtt_client) {
		if (mqtt_client) {
        	mosquitto_destroy(mqtt_client);
    	}
    	mosquitto_lib_cleanup();
	}
	
	digital_twin_client::digital_twin_client() {
		mqtt_client = nullptr;
		
		get_mosquitto_mqtt_version();
		
		init_mosquitto_lib();
		
		init_mqtt_client(&mqtt_client);
		
		init_user(mqtt_client);
	}
	
	digital_twin_client::~digital_twin_client() {
    	clean_up(mqtt_client);
	}

    void digital_twin_client::connect() {
		std::string broker_address = json_helper<std::string>::get_value_or_default("broker_address", "");
		int port = json_helper<int>::get_value_or_default("port", 0);
		int alive_time = json_helper<int>::get_value_or_default("alive_time", 0);
		
		int rc = mosquitto_connect(mqtt_client, broker_address.c_str(), port, alive_time);
		if (rc == MOSQ_ERR_SUCCESS) {
			std::cout << "Successfully connected to the broker." << std::endl;
		} else {
			std::cout << "Fail to connect to the broker: " << mosquitto_strerror(rc) << std::endl;
			clean_up(mqtt_client);
			exit(1);
		}
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
}


