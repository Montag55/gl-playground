#include <expansionMiddle.hpp>
#include <utils.hpp>
#include <structs.hpp>
#include <graphApp.hpp>
#include <expansionHandles.hpp>

ExpansionMiddle::ExpansionMiddle() {}

ExpansionMiddle::ExpansionMiddle(const int& leftAxisIndex, const int& rightAxisIndex, ExpansionHandles* leftHandle, ExpansionHandles* rightHandle, const GLuint& program, std::shared_ptr<GraphApp> app) :
    m_order{std::vector<int>{leftAxisIndex, rightAxisIndex}},
    m_leftHandle{leftHandle},
    m_rightHandle{rightHandle},
    m_program{program},
    m_linkedApp{app}
{
    m_axis = std::vector<float>{m_leftHandle->getHandlePos(), m_rightHandle->getHandlePos()};
    m_attributeCount = m_linkedApp->getAxis()->size();
    m_lineCount = m_linkedApp->getData()->size() / m_attributeCount / *m_linkedApp->getNumTimeAxis();
    m_leftDepthIndex = *m_linkedApp->getNumTimeAxis() * m_leftHandle->getHandleTime();
    m_rightDepthIndex = *m_linkedApp->getNumTimeAxis() * m_rightHandle->getHandleTime();

    // init with left and right handle
    m_times.push_back(m_leftDepthIndex);
    m_times.push_back(m_rightDepthIndex);
	
    initializeVertexBuffers();
    initializeIndexBuffers();
    updateContainedAxis();
    initializeStorageBuffers();
}

ExpansionMiddle::~ExpansionMiddle() {
    if (glIsBuffer(m_ibo)) {
        glDeleteBuffers(1, &m_ibo);
    }
}

void ExpansionMiddle::initializeIndexBuffers() {
    for (unsigned short i = 0; i < m_vertices.size(); i++) {
        m_indicies.push_back(i);
    }

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    // allocate max possible space needed (all indicies -> normal view indecies)
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(*m_linkedApp->getIndicies()), NULL, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_DYNAMIC_DRAW);
}

void ExpansionMiddle::updateAxisOrder(const std::vector<int>& axisIndicies) const {
    return;
    // return early if there is no actuall update
    auto prev = std::vector<int>{m_order.begin() + 1, m_order.end() - 1};
    if (prev == axisIndicies)
        return;
       
    // gets called whenever axis is added or removed
    // ToDo: sort out axis ordring -> keed left and right as start and end
    ExpansionMiddle* ptr = const_cast<ExpansionMiddle*>(this);
    ptr->m_order.erase(m_order.begin() + 1, m_order.end() - 1);
    ptr->m_order.insert(m_order.begin() + 1, axisIndicies.begin(), axisIndicies.end());
    updateContainedAxis();
}

void ExpansionMiddle::updateContainedAxis(const bool& init) const {
    return;

    /** funktion has to be called:
     *  1. to initialize middle
     *  2. when new axis is added to middle
     *  3. when axis is removed from middle
     *  4. when order of middle axis changes (?)
    **/   

    ExpansionMiddle* ptr = const_cast<ExpansionMiddle*>(this);
    ptr->m_indicies.clear();

    // add vertecies and indicies per line
    for (unsigned short i = 0; i < m_lineCount * m_order.size(); i++) {
        // add every indicie twice if it isnt start or endpoint of line
        unsigned short idx = i / m_axis.size() * m_attributeCount + m_order[i % m_order.size()];
        ptr->m_indicies.push_back(idx);
        if (i % m_order.size() != 0 && i % m_order.size() != m_order.size() - 1) {
            ptr->m_indicies.push_back(idx);
        }
    }

    glNamedBufferSubData(m_ibo, 0, Utils::vectorsizeof(m_indicies), m_indicies.data());
}

void ExpansionMiddle::initializeStorageBuffers() {
    auto axis = *m_linkedApp->getAxis();
    for (int i = 0; i < m_axis.size(); i++) {
        axis[m_order[i]] = m_axis[i];
    }

    GLuint attribute_pos_binding = 3;
    glCreateBuffers(1, &m_axis_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, attribute_pos_binding, m_axis_ssbo);
    glNamedBufferData(m_axis_ssbo, Utils::vectorsizeof(axis), axis.data(), GL_DYNAMIC_DRAW);
        
    GLuint times_binding = 5;
    glCreateBuffers(1, &m_times_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, times_binding, m_times_ssbo);
    glNamedBufferData(m_times_ssbo, Utils::vectorsizeof(m_times), m_times.data(), GL_DYNAMIC_DRAW);
}

void ExpansionMiddle::initializeVertexBuffers() {
    /**
    *   Only passing in floats since int and uint seems to have padding / offset errors.
    *   ID and AttribueIndex are casted to int in shader           
    */
        
    // setup vertex array object and vertex buffer
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));
    
    // setup value id attribute
    GLuint id_attrib_idx = 0;
    glEnableVertexArrayAttrib(m_vao, id_attrib_idx);
    glVertexArrayAttribFormat(m_vao, id_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, id));
    glVertexArrayAttribBinding(m_vao, id_attrib_idx, 0);

    // setup attribute index id attribute
    GLuint att_attrib_idx = 1;
    glEnableVertexArrayAttrib(m_vao, att_attrib_idx);
    glVertexArrayAttribFormat(m_vao, att_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, attIndx));
    glVertexArrayAttribBinding(m_vao, att_attrib_idx, 0);

    GLuint time_attrib_idx = 2;
    glEnableVertexArrayAttrib(m_vao, time_attrib_idx);
    glVertexArrayAttribFormat(m_vao, time_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, timeIndx));
    glVertexArrayAttribBinding(m_vao, time_attrib_idx, 0);

    // setup vertices
    for (int i = 0; i < m_lineCount; i++) {
        for (int j = 0; j < m_order.size(); j++) {            
            m_vertices.push_back( Vertex{
                float(i),
                float(m_order[j]),
                float(j)
            });
        }
    }

    glNamedBufferData(m_vbo, Utils::vectorsizeof(m_vertices), m_vertices.data(), GL_DYNAMIC_DRAW);
}

void ExpansionMiddle::updateAxisPos() const {
    auto axis = *m_linkedApp->getAxis();
    for (int i = 0; i < m_axis.size(); i++) {
        axis[m_order[i]] = m_axis[i];
    }
    
    // revert transform of screenspace for first and last (handles that already have transform applyed)
    axis[m_order[0]] *= 1 / m_linkedApp->getModel()[0][0];
    axis[m_order[m_order.size() - 1]] *= 1 / m_linkedApp->getModel()[0][0];
    glNamedBufferSubData(m_axis_ssbo, 0, Utils::vectorsizeof(axis), axis.data());

}

void ExpansionMiddle::updateTimeStorageBuffer() const {
    ExpansionMiddle* ptr = const_cast<ExpansionMiddle*>(this);
    ptr->m_times.clear();
    float deltaTime = m_leftDepthIndex - m_rightDepthIndex;
    for (int i = 0; i < m_axis.size(); i++) {
        // determin linear time between left and right handle
        ptr->m_times.push_back(m_leftDepthIndex + (i / (m_axis.size() - 1)) * -deltaTime);
    }
    
    glNamedBufferSubData(m_times_ssbo, 0, Utils::vectorsizeof(m_times), m_times.data());
}

void ExpansionMiddle::updateHandles() const {
    auto leftHandlePos = m_leftHandle->getHandlePos();
    auto rightHandlePos = m_rightHandle->getHandlePos();
    
    // update axis pos of left and right handles of position changed
    // if no change in screenspace -> return early
    if (leftHandlePos == m_axis[0] && rightHandlePos == m_axis[m_axis.size() - 1]) {
        return;
    }
    
    // adjust axis pos in ssbo
    ExpansionMiddle* ptr = const_cast<ExpansionMiddle*>(this);
    ptr->m_axis[0] = leftHandlePos;
    ptr->m_axis[m_axis.size() - 1] = rightHandlePos;
    updateAxisPos();
    
    
    // check specificly for depth change
    auto leftHandleTime = (*m_linkedApp->getNumTimeAxis() - 1) * m_leftHandle->getHandleTime();
    auto rightHandleTime = (*m_linkedApp->getNumTimeAxis() - 1) * m_rightHandle->getHandleTime();
    // if no change of axis depth -> retrun early;
    if (leftHandleTime == m_leftDepthIndex && rightHandleTime == m_rightDepthIndex)
        return;

    ptr->m_leftDepthIndex = leftHandleTime;
    ptr->m_rightDepthIndex = rightHandleTime;
    updateTimeStorageBuffer();
}

void ExpansionMiddle::draw() const {
    // update axis if handles moved
    updateHandles();

    glUseProgram(m_program);

    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "transform"), m_linkedApp->getModel());
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "to_range"), glm::vec2(-1, 1));
    glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_attributes"), m_linkedApp->getAxis()->size());
    glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_data"), m_linkedApp->getData()->size() / *m_linkedApp->getNumTimeAxis());
    glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_time"), *m_linkedApp->getNumTimeAxis());

    // bind VAO with all vertecies in there
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_axis_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_times_ssbo);

    // tell tesellation shader how many verts per line
    glPatchParameteri(GL_PATCH_VERTICES, 2);

    // uses buffer currently bound to GL_ELEMENT_ARRAY_BUFFER
    glDrawElements(GL_PATCHES, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*)0);
}