#pragma once

#include <vector>
#include <string>

namespace digital_twin {
	class utility_functions {
		public:
			static std::vector<std::string> split_text(std::string text, char delimiter);
	};
}
