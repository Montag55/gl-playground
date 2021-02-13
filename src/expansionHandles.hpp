#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <gl/program.hpp>
#include <gl/shader.hpp>
#include <functional>


struct AxisVertex;
struct NonAABB;
class GraphApp;

class ExpansionHandles {
public:
    ExpansionHandles();
    ExpansionHandles(const glm::mat4& model, const GLuint& program, std::shared_ptr<GraphApp> app);
    ~ExpansionHandles();
    void updateVertecies(const glm::mat4& model) const;
    bool updateSelection(const glm::vec2& prev, const glm::vec2& current, const float& angle) const;
    void draw() const;
    void checkSelection();
    const float getHandleTime();
    const float getHandlePos();

private:
    void initializeVertexBuffers();
    void initializeIndexBuffer();
    
    glm::mat4 m_model;
	GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_axis_ssbo;    
    
    float m_handle;
    float m_thickness;
    bool m_active;
    float m_pos;
    std::vector<AxisVertex> m_vertices;
    std::vector<unsigned short> m_indicies;
    glm::vec4 m_default_color;
    glm::vec4 m_active_color;
    std::unique_ptr<NonAABB> m_bb;
    std::shared_ptr<GraphApp> m_linkedApp;
};