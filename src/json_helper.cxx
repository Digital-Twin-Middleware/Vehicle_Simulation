#include <string>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "json_helper.h"
#include "utility_functions.h"

using json = nlohmann::json;

namespace digital_twin {
	
	template<>
	json json_helper<json>::config = json{}; 
	
	template<>
	json json_helper<std::string>::config = json{}; 
	
	template<>
	json json_helper<bool>::config = json{};
	
	template<>
	json json_helper<int>::config = json{};
	
	const std::string json_config_file_path = "config.json";
	
	json initialize_config(std::string json_config_file_path) {
		json config;
		std::fstream file(json_config_file_path);
		if (!file) {
			std::cerr << "Unable to open the config file." << std::endl;
			exit(1);
		}
		
		try {
			config = json::parse(file);
		} catch (json::parse_error& e) {
			std::cerr << "Unable to parse the config file in to json." << std::endl;
			exit(1);
		}
		
		return config;
	}
	
		
	template <typename T>
	T json_helper<T>::get_value_or_default(std::string key, T default_value) {
		if (config.empty()) {
			config = initialize_config(json_config_file_path);
		}
		try {
			std::vector<std::string> keys = utility_functions::split_text(key, '/');
			json temp = config;
			
			for (std::string k : keys) {
				if (temp.contains(k)) temp = temp.at(k);
				else throw json::out_of_range::create(403, "Key not found: " + k, nullptr);
			}
			
			return temp.get<T>();
		}
		catch (const nlohmann::json::out_of_range& e) {
			std::cerr << "Key not found: " << e.what() << std::endl;
		}
		catch (const nlohmann::json::type_error& e) {
			std::cerr << "Type mismatch: " << e.what() << std::endl;
		}
		
		return default_value;
	}
	
	template class json_helper<std::string>;
	template class json_helper<json>;
	template class json_helper<bool>;
	template class json_helper<int>;
}
