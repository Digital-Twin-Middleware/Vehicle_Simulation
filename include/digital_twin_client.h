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
			int file_publishing_chunk_size;
        public:
        	digital_twin_client();
        	~digital_twin_client();
          	void connect();
            void disconnect();
            struct publish_setting construct_publish_setting(std::string topic_key, std::string qos_key, std::string retain_key);
            int publish_message(std::string message, struct publish_setting setting);
            int publish_file(std::string file_path, struct publish_setting setting);
    };
}
