#pragma once

#include <functional>

#include "digital_twin_client.h"

namespace digital_twin {
	struct vector_2 {
		float x;
		float z;
		
		struct vector_2 operator+(const vector_2 &other) const {
			return vector_2 { 
				x + other.x,
				z + other.z
			};
		}
		
		struct vector_2 operator-(const vector_2 &other) const {
			return vector_2 { 
				x - other.x,
				z - other.z
			};
		}
	};
	
	enum car_status {
		IDLE = 0,
		RUNNING = 1,
		FINISH = 2
	};
	
	class car_simulation {
		private:
			std::function<void(struct mosquitto_message)> message_received_event;
			
			void on_message_received(struct mosquitto_message message);
		public:
			struct vector_2 position;
			struct vector_2 desired_direction;
			struct vector_2 direction;
			float velocity;
			bool is_turning;
			car_status status;
			
			
			car_simulation();
			void register_data(digital_twin_client &client);
			void run(float delta_time, digital_twin_client &client);
	};
}
