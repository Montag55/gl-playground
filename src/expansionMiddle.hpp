#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <gl/program.hpp>
#include <gl/shader.hpp>
#include <functional>


struct Vertex;
class GraphApp;

class ExpansionMiddle {
public:
	ExpansionMiddle();
	ExpansionMiddle(const int& leftAxisIndex, const int& rightAxisIndex, const int& lineCount, const int& attributeCount, const GLuint& program, GraphApp* app);
	~ExpansionMiddle();
	void update(const bool& init = false) const;
	void draw() const;
    void updateAxis(const std::vector<int>& axisIndicies) const;

private:
    void initializeIndexBuffers();
    
	GLuint m_program;
    GLuint m_ibo;

	std::vector<float> m_axis;
	std::vector<int> m_order;
    std::vector<unsigned short> m_indicies;
	
	int m_leftDepthIndex; // if 0, left handle between left axis [0,1], if 1 -> [1,2] ...
	int m_rightDepthIndex; // if 0, left handle between left axis [0,1], if 1 -> [1,2] ...
	int m_lineCount;
	int m_attributeCount;

	std::shared_ptr<GraphApp> m_linkedApp;
};
