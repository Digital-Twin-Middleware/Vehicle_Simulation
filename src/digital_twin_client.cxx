#include <string>
#include <iostream>
#include <mosquitto.h>
#include <nlohmann/json.hpp>

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
	
	void init_mqtt_client(struct mosquitto *mqtt_client) {
		std::string client_id = json_helper<std::string>::get_value_or_default("client_id", "Undefined_");
		std::cout << client_id << std::endl;
 	}
 	
 	void init_connect_options() {
 		/*connect_options = mqtt::connect_options("vuong", "Quocanh01");
       	connect_options.set_keep_alive_interval(20);
       	connect_options.set_connect_timeout(5);
        connect_options.set_clean_session(true);*/
 	}
	
	digital_twin_client::digital_twin_client() {
		get_mosquitto_mqtt_version();
		
		mosquitto_lib_init();
		
		init_mqtt_client(mqtt_client);
		
		/*init_connect_options(connect_options);
		
		if (mqtt_client) {
			std::cout << "Client is not null" << std::endl;
			
			std::cout << "User name: " << connect_options.get_user_name() << std::endl;
			std::cout << "Password: " << connect_options.get_password() << std::endl;
		}*/
	}

    void digital_twin_client::connect() {
        /*try {
            digital_twin_client_callback callback;
            mqtt_client.set_callback(callback);

            mqtt_client.connect(connOpts)->wait();
        }
        catch (const mqtt::exception& ex) {
            std::cerr << "MQTT Exception: " << ex.what() << std::endl;
        }*/
    }
    
    void digital_twin_client::disconnect() {
    	/*if (mqtt_client.is_connected()) {
    		mqtt_client.disconnect()->wait();
    	}*/
    }
}


