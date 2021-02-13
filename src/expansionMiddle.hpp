#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <gl/program.hpp>
#include <gl/shader.hpp>
#include <functional>


struct Vertex;
class GraphApp;
class ExpansionHandles;

class ExpansionMiddle {
public:
	ExpansionMiddle();
	ExpansionMiddle(const int& leftAxisIndex, const int& rightAxisIndex, ExpansionHandles* leftHandle, ExpansionHandles* rightHandle, const GLuint& program, std::shared_ptr<GraphApp> app);
	~ExpansionMiddle();
    void updateContainedAxis(const bool& init = false) const;
	void draw() const;
    void updateAxisOrder(const std::vector<int>& axisIndicies) const;

private:
    void updateHandles() const;
	void updateAxisPos() const;
    void updateTimeStorageBuffer() const;
    void initializeIndexBuffers();
    void initializeStorageBuffers();
    void initializeVertexBuffers();
    
	GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
	GLuint m_axis_ssbo;
	GLuint m_times_ssbo;

    std::vector<Vertex> m_vertices;
	std::vector<float> m_axis;
	std::vector<float> m_times;
	std::vector<int> m_order;
    std::vector<unsigned short> m_indicies;
	
	float m_leftDepthIndex; // if 0, left handle between left axis [0,1], if 1 -> [1,2] ...
	float m_rightDepthIndex; // if 0, left handle between left axis [0,1], if 1 -> [1,2] ...
	int m_lineCount;
	int m_attributeCount;

	std::shared_ptr<GraphApp> m_linkedApp;
	std::unique_ptr<ExpansionHandles> m_leftHandle;
	std::unique_ptr<ExpansionHandles> m_rightHandle;
};
