#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "json_helper.h"

using json = nlohmann::json;

namespace digital_twin {
	std::string json_helper::json_config_file_path = "config.json";
	json json_helper::config = json{}; 
	
	json json_helper::get_config() {
		if (!config.empty()) return config;
		
		std::fstream file(json_config_file_path);
		
		if (!file) {
			std::cerr << "Unable to open the config file." << std::endl;
			return json{};
		}
		
		try {
			config = json::parse(file);
		} catch (json::parse_error& e) {
			std::cerr << "Unable to parse the config file in to json." << std::endl;
			return json{};
		}
		
		return config;
	}
}
