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
		IDLE,
		RUNNING,
		FINISH
	};
	
	class car_simulation {
		private:
			struct vector_2 position;
			struct vector_2 direction;
			struct vector_2 desired_direction;
			float velocity;
			float desired_velocity;
			float acceleration;
			car_status status;
			std::function<void(struct mosquitto_message)> message_received_event;
			
			void on_message_received(struct mosquitto_message message);
		public:
			car_simulation();
			void register_data(digital_twin_client &client);
			void run(float delta_time);
	};
}
