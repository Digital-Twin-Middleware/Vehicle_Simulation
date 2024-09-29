#pragma once

#include <string>
#include <memory>
#include <mosquitto.h>

namespace digital_twin {
    class digital_twin_client
    {
    	private:
			struct mosquitto *mqtt_client;    		
        public:
        	digital_twin_client();
          	void connect();
            void disconnect();
    };
}
