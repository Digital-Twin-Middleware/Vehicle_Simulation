#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace digital_twin {
	class json_helper 
	{
		private:
			static std::string json_config_file_path;
			static json config;
		public:
			static json get_config();
	};
}

