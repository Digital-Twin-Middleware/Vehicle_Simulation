#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace digital_twin {
	template <typename T>
	class json_helper 
	{
		private:
			static json config;
		public:
			static T get_value_or_default(std::string key, T default_value);
	};
}

