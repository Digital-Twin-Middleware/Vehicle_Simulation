#pragma once

#include <string>
#include <memory>
#include <mosquitto.h>

namespace digital_twin {
	struct publish_setting {
		std::string topic;
		int qos_level;
		bool retain;
	};
	
    class digital_twin_client
    {
    	private:
			struct mosquitto *mqtt_client;	
			struct publish_setting setting;
        public:
        	digital_twin_client();
        	~digital_twin_client();
          	void connect();
            void disconnect();
            void publish(std::string message);
    };
}
