#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <gl/program.hpp>
#include <gl/shader.hpp>
#include <functional>

struct Vertex;
class GraphApp;

class ExpansionActive {
public:
    ExpansionActive();
	ExpansionActive(const int& leftIdx, const int& rightIdx, const GLuint& program, GraphApp* app);
	~ExpansionActive();
    void draw() const;
    void setActive(const bool& state) const;

private:
    void initializeVertexBuffers();
    void initializeIndexBuffer();
    
    glm::mat4 m_model;
	GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_axis_ssbo;

    std::vector<Vertex> m_vertices;
    std::vector<unsigned short> m_indicies;
    float m_thickness;
    glm::vec4 m_default_color;
    glm::vec4 m_active_color;

    int m_leftAxisIndex;
    int m_rightAxisIndex;
    bool m_active;

    std::shared_ptr<GraphApp> m_linkedApp;
};