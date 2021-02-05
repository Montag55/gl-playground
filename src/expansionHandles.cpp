#include <expansionHandles.hpp>
#include <utils.hpp>
#include <structs.hpp>
#include <graphApp.hpp>

ExpansionHandles::ExpansionHandles() {}

ExpansionHandles::ExpansionHandles(const int& index, const glm::mat4& model, const GLuint& program) : 
    m_program{program},
    m_axisIndex{index}
{  
    m_thickness = 0.05f;
    m_handle = 0.5f;
    m_active_color = glm::vec4(0.180, 0.215, 0.298, 1);
	
    initializeVertexBuffers();
	initializeIndexBuffer();
    updateVertecies(model);
}

void ExpansionHandles::draw() const {
    glUseProgram(m_program);
    
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "active_color"), m_active_color);
    
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    glDrawElements(GL_TRIANGLE_STRIP, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*)0);
}

ExpansionHandles::~ExpansionHandles() {
    if (glIsBuffer(m_vbo)) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (glIsVertexArray(m_vao)) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (glIsBuffer(m_ibo)) {
        glDeleteBuffers(1, &m_ibo);
    }
}

void ExpansionHandles::initializeVertexBuffers() {
	// setup vertex array object and vertex buffer
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(AxisVertex));

    GLuint pos_attrib_idx = 0;
    glEnableVertexArrayAttrib(m_vao, pos_attrib_idx);
    glVertexArrayAttribFormat(m_vao, pos_attrib_idx, 2, GL_FLOAT, false, offsetof(AxisVertex, pos));
    glVertexArrayAttribBinding(m_vao, pos_attrib_idx, 0);

    GLuint color_attrib_idx = 1;
    glEnableVertexArrayAttrib(m_vao, color_attrib_idx);
    glVertexArrayAttribFormat(m_vao, color_attrib_idx, 1, GL_FLOAT, false, offsetof(AxisVertex, colorIndx));
    glVertexArrayAttribBinding(m_vao, color_attrib_idx, 0);

    glNamedBufferData(m_vbo, 4 * sizeof(AxisVertex), NULL, GL_DYNAMIC_DRAW);
}

void ExpansionHandles::updateVertecies(const glm::mat4& model) const {

    // determin y-coords for depth scaling
    float right_val = Utils::remap(1 - m_handle - m_thickness / 2, glm::vec2(0, 1), glm::vec2(0.125 * (1 + m_thickness), 1 + m_thickness));
    float left_val = Utils::remap(1 - m_handle + m_thickness / 2, glm::vec2(0, 1), glm::vec2(0.125 * (1 + m_thickness), 1 + m_thickness));

    /** init indecies for MultiDrawArrays call
     * 0 +----+ 1
     *   |    |
     *   |    |
     * 2 +----+ 3
    **/
    ExpansionHandles* ptr = const_cast<ExpansionHandles*>(this);
    ptr->m_vertices.clear();
    ptr->m_vertices.push_back(AxisVertex{model * glm::vec4(m_handle - m_thickness / 2, left_val, 0.0f, 1.0f), 0});
    ptr->m_vertices.push_back(AxisVertex{model * glm::vec4(m_handle + m_thickness / 2, right_val, 0.0f, 1.0f), 0});
    ptr->m_vertices.push_back(AxisVertex{model * glm::vec4(m_handle - m_thickness / 2, -left_val, 0.0f, 1.0f), 0});
    ptr->m_vertices.push_back(AxisVertex{model * glm::vec4(m_handle + m_thickness / 2, -right_val, 0.0f, 1.0f), 0});

    glNamedBufferSubData(m_vbo, 0, Utils::vectorsizeof(m_vertices), m_vertices.data());
}

void ExpansionHandles::initializeIndexBuffer() {
    m_indicies = std::vector<unsigned short> {0,1,2,3};
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_STATIC_DRAW);
}

