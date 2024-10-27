#pragma once

#include <vector>
#include <string>
#include <chrono>

namespace digital_twin {
	class utility_functions {
		private:
			std::chrono::steady_clock::time_point current_time;
			float time_per_frame;
			float current_time_frame;
			bool passed;
		public:
			utility_functions();
			float get_delta_time();
			bool is_pass_frame();
			static std::vector<std::string> split_text(std::string text, char delimiter);
			static int get_random_in_range(int min, int max);
	};
}
