#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <gl/program.hpp>
#include <gl/shader.hpp>
#include <functional>

struct TimeExpansion;
struct AxisVertex;
enum Handle;
class GraphApp;

class ExpansionHandles {
public:
    ExpansionHandles();
    ExpansionHandles(const int& index, const glm::mat4& model, const GLuint& program);
    ~ExpansionHandles();
    void updateVertecies(const glm::mat4& model) const;
    void draw() const;

private:
    void initializeVertexBuffers();
    void initializeIndexBuffer();

	GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_axis_ssbo;
    glm::mat4 m_model;

    std::vector<AxisVertex> m_vertices;
    std::vector<unsigned short> m_indicies;
    float m_handle;
    float m_thickness;
    int m_axisIndex;
    glm::vec4 m_default_color;
    glm::vec4 m_active_color;

};