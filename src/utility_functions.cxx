#include <vector>
#include <string>
#include <sstream>
#include <chrono>

#include "utility_functions.h"

namespace digital_twin {
    std::chrono::steady_clock::time_point utility_functions::current_time = std::chrono::steady_clock::now();

	void utility_functions::set_timer() {
		current_time = std::chrono::steady_clock::now();
	}
	
	float utility_functions::get_delta_time() {
		auto new_time = std::chrono::steady_clock::now();
		
		float delta_time = std::chrono::duration<float>(new_time - current_time).count();
		
		current_time = new_time;
		
		return delta_time;
	}

	std::vector<std::string> utility_functions::split_text(std::string text, char delimiter) {
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream token_stream(text);
		while (std::getline(token_stream, token, delimiter)) {
			tokens.push_back(token);
		}
		return tokens;
	}
}
