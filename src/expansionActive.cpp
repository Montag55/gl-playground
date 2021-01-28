#include <expansionActive.hpp>
#include <utils.hpp>
#include <structs.hpp>
#include <graphApp.hpp>

ExpansionActive::ExpansionActive() {}

ExpansionActive::ExpansionActive(const int& leftIdx, const int& rightIdx, const GLuint& program, std::shared_ptr<GraphApp> app) : 
	m_linkedApp{app},
    m_program{program},
    m_leftAxisIndex{leftIdx},
    m_rightAxisIndex{rightIdx}
{  
    m_active = false;
    m_thickness = 0.05f;
    m_model = m_linkedApp->getModel();
    m_active_color = glm::vec4(1, 1, 1, 0.025f);
	
    initializeVertexBuffers();
	initializeIndexBuffer();
}

void ExpansionActive::draw() const {
    if (!m_active)
        return;

    glUseProgram(m_program);
    
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "transform"), m_model);
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "active_color"), m_active_color);
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "thickness_offset"), glm::vec4(m_thickness, -m_thickness, m_thickness, -m_thickness));
     
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, *m_linkedApp->getAttribute_SSBO());

    glDrawElements(GL_TRIANGLE_STRIP, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*)0);
}

ExpansionActive::~ExpansionActive() {
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

void ExpansionActive::initializeVertexBuffers() {
	// setup vertex array object and vertex buffer
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
    
    // setup axis id attribute
    GLuint axis_attrib_idx = 0;
    glEnableVertexArrayAttrib(m_vao, axis_attrib_idx);
    glVertexArrayAttribFormat(m_vao, axis_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, id));
    glVertexArrayAttribBinding(m_vao, axis_attrib_idx, 0);

    // setup y coord attribute
    GLuint y_coord_attrib_idx = 1;
    glEnableVertexArrayAttrib(m_vao, y_coord_attrib_idx);
    glVertexArrayAttribFormat(m_vao, y_coord_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, attIndx));
    glVertexArrayAttribBinding(m_vao, y_coord_attrib_idx, 0);

    m_vertices = std::vector<Vertex>{
		Vertex{float(m_leftAxisIndex), 1.05f},
		Vertex{float(m_rightAxisIndex), 1.05f},
		Vertex{float(m_leftAxisIndex), -1.05f},
		Vertex{float(m_rightAxisIndex), -1.05f}
	};

    glNamedBufferData(m_vbo, Utils::vectorsizeof(m_vertices), m_vertices.data(), GL_DYNAMIC_DRAW);
}

void ExpansionActive::initializeIndexBuffer() {
    m_indicies = std::vector<unsigned short> {0,1,2,3};
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_STATIC_DRAW);
}

void ExpansionActive::setActive(const bool& state) const {
    ExpansionActive* ptr = const_cast<ExpansionActive*>(this);
    ptr->m_active = state;
}

