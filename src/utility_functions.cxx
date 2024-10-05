#include <vector>
#include <string>
#include <sstream>

#include "utility_functions.h"

namespace digital_twin {
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
