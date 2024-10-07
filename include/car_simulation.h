#pragma once

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
	
	class car_simulation {
		private:
			struct vector_2 position;
			struct vector_2 direction;
			struct vector_2 desired_direction;
			float velocity;
			float desired_velocity;
			bool has_started;
			bool has_finished;
		public:
			car_simulation();
			void register_data(digital_twin_client &client);
			void run(float delta_time);
	};
}
