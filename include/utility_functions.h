#pragma once

#include <vector>
#include <string>
#include <chrono>

namespace digital_twin {
	class utility_functions {
		private:
			static std::chrono::steady_clock::time_point current_time;
		public:
			static void set_timer();
			static float get_delta_time();
			static std::vector<std::string> split_text(std::string text, char delimiter);
			static int get_random_in_range(int min, int max);
	};
}
