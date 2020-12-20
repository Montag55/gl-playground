#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

namespace Utils{
    template<typename T>
    inline size_t vectorsizeof(const typename std::vector<T>& vec) {
        return sizeof(T) * vec.size();
    }
	
	inline float remap(float value, glm::vec2 from, glm::vec2 to) {
		return to.x + (value - from.x) * (to.y - to.x) / (from.y - from.x);
	}

	inline std::vector<std::string> splitString(const std::string& s,  const char& delimiter){
		size_t last = 0;
		size_t next = 0;
		std::vector<std::string> result;

		// find delimiter and push substring between them into vector
		while ((next = s.find(delimiter, last)) != std::string::npos) {
			result.push_back(s.substr(last, next - last));
			last = next + 1;
		}
	
		// push last substring not followed by delimiter into vector
		// result.push_back(s.substr(last, next - last));
	
		return result;
	}

    inline void readData(std::vector<float>& data, std::string path){
		/*
		 * Reads textfile specified by path then formats lines 
		 * and writes float data into the input data vector
		 */

		std::ifstream file(path);
		std::string line;
		while (std::getline(file, line)) {
			auto split = splitString(line, ',');
			
			// just skip invalid data for testing
			if (split.size() != 4) {
				continue;
			}
			
			for (const auto& i : split) {
				data.push_back(std::stof(i));
			}
		}
	}

	inline void getMaxValues(const std::vector<float>& data, glm::vec4& max) {
		for (int i = 0; i < data.size(); i+=4) {
			max.x = glm::max(data[i], max.x);
			max.y = glm::max(data[i+1], max.y);
			max.z = glm::max(data[i+2], max.z);
			max.w = glm::max(data[i+3], max.w);
		}
	}

	inline void getMinValues(const std::vector<float>& data, glm::vec4& min) {
		getMaxValues(data, min);
		for (int i = 0; i < data.size(); i+=4) {
			min.x = glm::min(data[i], min.x);
			min.y = glm::min(data[i+1], min.y);
			min.z = glm::min(data[i+2], min.z);
			min.w = glm::min(data[i+3], min.w);
		}
	}
}