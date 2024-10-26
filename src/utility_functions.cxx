#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <random>

#include "utility_functions.h"

namespace digital_twin {
	utility_functions::utility_functions() {
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
	
	int utility_functions::get_random_in_range(int min, int max) {
		if (max < min) max = min;
		
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distr(min, max);
		
		return distr(gen);
	}
}
