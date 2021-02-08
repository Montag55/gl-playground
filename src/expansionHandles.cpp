#include <expansionHandles.hpp>
#include <utils.hpp>
#include <structs.hpp>
#include <graphApp.hpp>


ExpansionHandles::ExpansionHandles() {}

ExpansionHandles::ExpansionHandles(const int& index, const glm::mat4& model, const GLuint& program, std::shared_ptr<GraphApp> app) : 
    m_program{program},
    m_axisIndex{index},
    m_bb{std::unique_ptr<NonAABB>(new NonAABB{})},
    m_linkedApp{app}
{  
    m_thickness = 0.05f;
    m_handle = 0.5f;
    m_active = false;
    m_default_color = glm::vec4(0.180, 0.215, 0.298, 0.2);
    m_active_color = glm::vec4(0.180, 0.215, 0.298, 0.4);
	
    initializeVertexBuffers();
	initializeIndexBuffer();
    updateVertecies(model);
}

void ExpansionHandles::draw() const {    
    glUseProgram(m_program);
    
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "active_color"), m_active_color);
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "default_color"), m_default_color);
    
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

void ExpansionHandles::checkSelection(){
    auto prev_state = m_active;
    auto current_pos = m_linkedApp->mouse_pos();
    float x = Utils::remap(current_pos.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1, 1));
    float y = Utils::remap(current_pos.y, glm::vec2(0, m_linkedApp->resolution().y), glm::vec2(-1, 1));
    
    if (Utils::insideNonAABB(glm::vec2(x, y), *m_bb))
        m_active = true;
    else
        m_active = false;    

    if (m_active != prev_state)
        updateVertecies(m_model);
}

bool ExpansionHandles::updateSelection(const glm::vec2& prev, const glm::vec2& current, const float& angle) const {
    // retrun early if mouse isnt over 
    if (!m_active)
        return false;

    float prog_dist = glm::sin(glm::radians(angle)) * m_linkedApp->getModel()[0][0];
    float prev_x = Utils::remap(prev.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1, 1));
    float current_x = Utils::remap(current.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1, 1));
    float delta_x = current_x - prev_x;
    
    float dist = delta_x / prog_dist;
    
    ExpansionHandles* ptr = const_cast<ExpansionHandles*>(this);
    ptr->m_handle = glm::clamp(m_handle - dist, 0.0f, 1.0f);
    updateVertecies(m_model);
    return true;
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
    ptr->m_model = model;
    ptr->m_vertices.clear();
    ptr->m_vertices.push_back(AxisVertex{m_model * glm::vec4(m_handle - m_thickness / 2, left_val, 0.0f, 1.0f), float(m_active)});
    ptr->m_vertices.push_back(AxisVertex{m_model * glm::vec4(m_handle + m_thickness / 2, right_val, 0.0f, 1.0f), float(m_active)});
    ptr->m_vertices.push_back(AxisVertex{m_model * glm::vec4(m_handle - m_thickness / 2, -left_val, 0.0f, 1.0f), float(m_active)});
    ptr->m_vertices.push_back(AxisVertex{m_model * glm::vec4(m_handle + m_thickness / 2, -right_val, 0.0f, 1.0f), float(m_active)});
    
    // set BoundingBox
    ptr->m_bb->uL = glm::vec2(m_vertices[0].pos.x, m_vertices[0].pos.y);
    ptr->m_bb->uR = glm::vec2(m_vertices[1].pos.x, m_vertices[1].pos.y);
    ptr->m_bb->lL = glm::vec2(m_vertices[2].pos.x, m_vertices[2].pos.y);
    ptr->m_bb->lR = glm::vec2(m_vertices[3].pos.x, m_vertices[3].pos.y);
    
    // order boudningbox, if left is right -> swap left with right
    if (m_bb->uL.x > m_bb->uR.x) {
        auto tmp = m_bb->uL;
        ptr->m_bb->uL = m_bb->uR;
        ptr->m_bb->uR = tmp;

        tmp = m_bb->lL;
        ptr->m_bb->lL = m_bb->lR;
        ptr->m_bb->lR = tmp;
    }

    glNamedBufferSubData(m_vbo, 0, Utils::vectorsizeof(m_vertices), m_vertices.data());
}

void ExpansionHandles::initializeIndexBuffer() {
    m_indicies = std::vector<unsigned short> {0,1,2,3};
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_STATIC_DRAW);
}

