#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <mosquitto.h>

namespace digital_twin {
	struct pubsub_setting {
		std::string topic;
		int qos_level;
		bool retain;
	};
	
    class digital_twin_client
    {
    	private:
			struct mosquitto *mqtt_client;
			int file_publishing_chunk_size;
			std::vector<std::function<void(struct mosquitto_message)>> message_received_callbacks;	
			void message_received(struct mosquitto *mqtt_client, void* user_data, const struct mosquitto_message *message);
        public:
        	digital_twin_client(int id);
          	void connect();
            void disconnect();
            struct pubsub_setting construct_pubsub_setting(int id, std::string topic_key, std::string qos_key, std::string retain_key);
            int publish_message(std::string message, struct pubsub_setting setting);
            int publish_file(std::string file_path, struct pubsub_setting setting);
            int subscribe(struct pubsub_setting setting);
            void register_message_received_callback(std::function<void(struct mosquitto_message)> callback);
            void unregister_message_received_callback(std::function<void(struct mosquitto_message)> callback);
            void invoke_message_received_event(struct mosquitto_message message);
    };
}
