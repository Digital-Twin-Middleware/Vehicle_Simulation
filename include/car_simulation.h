#pragma once

#include "digital_twin_client.h"

namespace digital_twin {
	class car_simulation {
		public:
			void register_car_data(digital_twin_client client);
	};
}
