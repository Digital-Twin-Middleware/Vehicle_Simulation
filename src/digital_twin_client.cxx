#include <string>
#include <iostream>
#include <mqtt/async_client.h>

#include "digital_twin_client.h"

namespace digital_twin {
    class digital_twin_client_callback : public virtual mqtt::callback {
        public:
            void connected(const std::string &cause) override {
                std::cout << "Connected: " << cause << std::endl;
            }

            void connection_lost(const std::string &cause) override {
                std::cout << "Connection lost: " << cause << std::endl;
            }
    };
	
	void init_mqtt_client(std::unique_ptr<mqtt::async_client> &mqtt_client) {
 		auto server_address = "b09b50fd24a44677bcd84f0669c31841.s1.eu.hivemq.cloud";
        auto client_id = "test_id";
		
 		mqtt_client = std::unique_ptr<mqtt::async_client>(new mqtt::async_client(server_address, client_id));
 	}
 	
 	void init_connect_options(mqtt::connect_options &connect_options) {
 		connect_options = mqtt::connect_options("vuong", "Quocanh01");
       	connect_options.set_keep_alive_interval(20);
       	connect_options.set_connect_timeout(5);
        connect_options.set_clean_session(true);
 	}
	
	digital_twin_client::digital_twin_client() {
		init_mqtt_client(mqtt_client);
		
		init_connect_options(connect_options);
		
		if (mqtt_client) {
			std::cout << "Client is not null" << std::endl;
			
			std::cout << "User name: " << connect_options.get_user_name() << std::endl;
			std::cout << "Password: " << connect_options.get_password() << std::endl;
		}
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


