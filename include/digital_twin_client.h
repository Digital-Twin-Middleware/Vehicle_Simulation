#pragma once

#include <string>
#include <memory>
#include <mqtt/async_client.h>

namespace digital_twin {
    class digital_twin_client
    {
    	private:
    		std::unique_ptr<mqtt::async_client> mqtt_client;
    		mqtt::connect_options connect_options;
        public:
        	digital_twin_client();
          	void connect();
            void disconnect();
    };
}
