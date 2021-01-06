#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm>
#include <structs.hpp>
#include <glm/glm.hpp>

namespace Utils{
    template<typename T>
    inline size_t vectorsizeof(const typename std::vector<T>& vec) {
        return sizeof(T) * vec.size();
    }
	
	inline float remap(const float& value, const glm::vec2& from, const glm::vec2& to) {
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

	inline std::vector<float> getMaxValues(const std::vector<float>& data, const int& num) {
		std::vector<float> max(4, 0);
		for (int i = 0; i < data.size(); i+=num) {
			max[0] = glm::max(data[i], max[0]);
			max[1] = glm::max(data[i+1], max[1]);
			max[2] = glm::max(data[i+2], max[2]);
			max[3] = glm::max(data[i+3], max[3]);
		}
		return max;
	}

	inline std::vector<float> getMinValues(const std::vector<float>& data, const int& num) {
		std::vector<float> min = getMaxValues(data, num);
		for (int i = 0; i < data.size(); i+=num) {
			min[0] = glm::min(data[i], min[0]);
			min[1] = glm::min(data[i+1], min[1]);
			min[2] = glm::min(data[i+2], min[2]);
			min[3] = glm::min(data[i+3], min[3]);
		}
		return min;
	}

	inline glm::vec2 bezier(const float& u, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) {
		float B0 = (1.0 - u) * (1.0 - u) * (1.0 - u);
		float B1 = 3.0 * (1.0 - u) * (1.0 - u) * u;
		float B2 = 3.0 * (1.0 - u) * u * u;
		float B3 = u * u * u;
		return B0 * p0 + B1 * p1 + B2 * p2 + B3 * p3;
	}

	inline bool intersectionAABB(const AABB& r1, const AABB& r2) {
		return !(r2.c1.x > r1.c2.x ||
				 r2.c2.x < r1.c1.x ||
				 r2.c1.y < r1.c2.y ||
				 r2.c2.y > r1.c1.y);
	}

	inline bool insideAABB(const glm::vec2& point, const AABB& rect) {
		if( rect.c1.x <= point.x && point.x <= rect.c2.x && rect.c2.y <= point.y && point.y <= rect.c1.y ) {
            return true;
        }
        return false;
	}

	inline double ellapsedTime(const std::chrono::time_point<std::chrono::system_clock>& start, const std::chrono::time_point<std::chrono::system_clock>& end) {
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}
	
	inline bool compare(const SortObj& a, const SortObj& b) { 
		return a.val < b.val; 
	} 

	inline bool sortWithIndecies(const std::vector<float>& val, std::vector<int>& order) {
		// create obj for linked sorting
		std::vector<SortObj> tmp;
		for (int i = 0; i < val.size(); i++) { 
			tmp.push_back(SortObj{
				val[i], 
				i
			}); 
		}
		
		// sort by value
		std::sort(tmp.begin(), tmp.end(), compare);
		
		// write linked sorting results to input buffers
		auto prv_val = val;
		for (int i = 0; i < tmp.size(); i++) {
			prv_val[i] = tmp[i].index;
			order[i] = tmp[i].index;
		}
		
		if(val != prv_val)
			return true;
		
		return false;
	}
}