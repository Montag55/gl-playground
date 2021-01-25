#include <axisDrag.hpp>

AxisDrag::AxisDrag(GraphApp* app) :
	Tool{app},
    m_axis_status{std::vector<bool>(m_linkedApp->getAxis()->size(), false)}
{
    auto vert_shader = gl::load_shader_from_file("shaders/axis.vert", GL_VERTEX_SHADER);
    auto frag_shader = gl::load_shader_from_file("shaders/axis.frag", GL_FRAGMENT_SHADER);
    m_program = gl::create_program({vert_shader, frag_shader});
	
    // model relative to Polylines scale
    m_draw_model = m_linkedApp->getModel();
    
    // model relative to screen space
    m_mouse_model = glm::scale(glm::mat4{1.0f}, glm::vec3{
        1.0f / m_linkedApp->getModel()[0][0], 
        1.0f / m_linkedApp->getModel()[1][1], 
        1.0f / m_linkedApp->getModel()[2][2]}
    );    
    
    // x coord span of axis rectangle
    m_thickness = 0.05f;
    m_default_color = glm::vec4(0.180, 0.215, 0.298, 0.2);
	m_active_color = glm::vec4(0.180, 0.215, 0.298, 0.4);
    
    // initialize axis order [0,1,2,3,...]
    for (int i = 0; i < m_linkedApp->getAxis()->size(); i++) {
        m_order.push_back(i);
    }
    
    initializeVertexBuffers();
    initializeIndexBuffer();
}

AxisDrag::~AxisDrag() {
    if (glIsBuffer(m_vbo)) {
        glDeleteBuffers(1, &m_vbo);
    }
    if (glIsVertexArray(m_vao)) {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (glIsBuffer(m_ibo)) {
        glDeleteBuffers(1, &m_ibo);
    }
    if (glIsProgram(m_program)) {
        glDeleteProgram(m_program);
    }
}

bool AxisDrag::draw() const {
    glUseProgram(m_program);
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "transform"), m_draw_model);
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "default_color"), m_default_color);
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "active_color"), m_active_color);
     
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glDrawElements(GL_TRIANGLE_STRIP, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*) 0);
    
    return true;
}

bool AxisDrag::registerTool() {
    return true;
}

bool AxisDrag::checkSelection() {
    auto pos = m_linkedApp->mouse_pos();
    float x = Utils::remap(pos.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1,1));
    float y = Utils::remap(pos.y, glm::vec2(0, m_linkedApp->resolution().y), glm::vec2(-1,1));
    auto ss_pos = m_mouse_model * glm::vec4(glm::vec2(x,y), 0, 1);
    
    std::vector<bool> current (m_axis_status.size(), false);
    
    // check for intersection with multiple axis
    bool result = false;
    for (int i = 0; i < m_axis_status.size(); i++) {
        AABB axis_rect = AABB {
            m_vertices[i * 4].pos,    // upper left
            m_vertices[i * 4 + 3].pos // lower right 
        };
        
        if (Utils::insideAABB(ss_pos, axis_rect)) {
            current[i] = true;
            result = true;
        }
        else {
            current[i] = false;
        }
    }
    
    // if there is a diffrence to last check update colors
    if (current != m_axis_status) {
        m_axis_status = current;
        updateColors();
    }

    return result;
}

void AxisDrag::updateColors() {
    for (int i = 0; i < m_vertices.size(); i++) {
        m_vertices[i].colorIndx = m_axis_status[(int)(i / 4)];
    }

    // sub only the 4 verts of selected axis
    glNamedBufferSubData(m_vbo, 0, Utils::vectorsizeof(m_vertices), m_vertices.data());
}

bool AxisDrag::updateSelection(const glm::vec2& prev, const glm::vec2& current) {
    bool moveing = false;
    auto axis = *m_linkedApp->getAxis();
    
    float scaled_prev_x = Utils::remap(prev.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1,1));
    float scaled_currnet_x = Utils::remap(current.x, glm::vec2(0, m_linkedApp->resolution().x), glm::vec2(-1,1));
    float x = scaled_currnet_x - scaled_prev_x;
   
    for (int i = 0; i < m_axis_status.size(); i++) {
        // move all selected axis by (mouse) delta
        if (m_axis_status[i]) {
            axis[i] += (m_mouse_model * glm::vec4(x, 0, 0, 1)).x;
            moveing = true;
        }
    }
    
    // re-sort axis since order might have changed
    if (Utils::sortWithIndecies(axis, m_order)) {
        // if order change, update vertices
        m_linkedApp->updateOrder(m_order);
    }
    
    // update verts (axis_ssbo) and axis (verts for axis rect)
    m_linkedApp->updateAxis(axis);
    updateAxis(axis);
    
    return moveing;
}

void AxisDrag::updateAxis(const std::vector<float>& axis) {
    m_vertices.clear();
    for (const auto& i : axis) {
        m_vertices.push_back(AxisVertex{glm::vec2(i - m_thickness / 2,  1.05), 0});
        m_vertices.push_back(AxisVertex{glm::vec2(i + m_thickness / 2,  1.05), 0});
        m_vertices.push_back(AxisVertex{glm::vec2(i - m_thickness / 2, -1.05), 0});
        m_vertices.push_back(AxisVertex{glm::vec2(i + m_thickness / 2, -1.05), 0});
    }

    glNamedBufferSubData(m_vbo, 0, Utils::vectorsizeof(m_vertices), m_vertices.data());
}

void AxisDrag::initializeVertexBuffers() {
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

    /** init indecies for MultiDrawArrays call
     * 0 +----+ 1
     *   |    |
     *   |    |
     * 2 +----+ 3
    **/
    for (const auto& i : *m_linkedApp->getAxis()) {
        m_vertices.push_back(AxisVertex{glm::vec2(i - m_thickness / 2,  1.05), 0});
        m_vertices.push_back(AxisVertex{glm::vec2(i + m_thickness / 2,  1.05), 0});
        m_vertices.push_back(AxisVertex{glm::vec2(i - m_thickness / 2, -1.05), 0});
        m_vertices.push_back(AxisVertex{glm::vec2(i + m_thickness / 2, -1.05), 0});
    }  

    glNamedBufferData(m_vbo, Utils::vectorsizeof(m_vertices), m_vertices.data(), GL_DYNAMIC_DRAW);
}

void AxisDrag::initializeIndexBuffer() {
    for (unsigned short i = 0; i < m_vertices.size(); i++) {
        m_indicies.push_back(i);
        
        // create degenerate triangles between quads
        if ((i % 4 == 3 || i % 4 == 0) && i != 0 && i != m_vertices.size() - 1)
            m_indicies.push_back(i);
    }
    
    // Bind to Element array buffer -> Indexing so DrawElements can be used
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_STATIC_DRAW);
}