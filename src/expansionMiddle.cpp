#include <expansionMiddle.hpp>
#include <utils.hpp>
#include <structs.hpp>
#include <graphApp.hpp>

ExpansionMiddle::ExpansionMiddle() {}

ExpansionMiddle::ExpansionMiddle(const int& leftAxisIndex, const int& rightAxisIndex, const int& lineCount, const int& attributeCount, const GLuint& program, GraphApp* app) :
	m_leftDepthIndex{0}, 
	m_rightDepthIndex{0},
	m_axis{std::vector<float>{-1, 1}},
    m_order{std::vector<int>{leftAxisIndex, rightAxisIndex}},
    m_lineCount{lineCount},
    m_attributeCount{attributeCount},
    m_program{program},
    m_linkedApp{app}
{
	initializeIndexBuffers();
    update();
}

ExpansionMiddle::~ExpansionMiddle() {
    if (glIsBuffer(m_ibo)) {
      glDeleteBuffers(1, &m_ibo);
    }
}

void ExpansionMiddle::initializeIndexBuffers() {
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    // allocate max possible space needed (all indicies -> normal view indecies)
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(*m_linkedApp->getIndicies()), NULL, GL_DYNAMIC_DRAW);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_DYNAMIC_DRAW);
}

void ExpansionMiddle::updateAxis(const std::vector<int>& axisIndicies) const {
    // return early if there is no actuall update
    auto prev = std::vector<int>{m_order.begin() + 1, m_order.end() - 1};
    if (prev == axisIndicies)
        return;
    
    // gets called whenever axis is added or removed
    // ToDo: sort out axis ordring -> keed left and right as start and end
    ExpansionMiddle* ptr = const_cast<ExpansionMiddle*>(this);
    ptr->m_order.erase(m_order.begin() + 1, m_order.end() - 1);
    ptr->m_order.insert(m_order.begin() + 1, axisIndicies.begin(), axisIndicies.end());
    update();
}

void ExpansionMiddle::update(const bool& init) const {
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

void ExpansionMiddle::draw() const {
    glUseProgram(m_program);

    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "transform"), glm::scale(glm::mat4{1.0f}, glm::vec3{0.8f}));
    gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "to_range"), glm::vec2(-1, 1));
    glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_attributes"), m_linkedApp->getAxis()->size());

    // bind VAO with all vertecies in there
    glBindVertexArray(*m_linkedApp->getVAO());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);

    // tell tesellation shader how many verts per line
    glPatchParameteri(GL_PATCH_VERTICES, 2);

    // uses buffer currently bound to GL_ELEMENT_ARRAY_BUFFER
    glDrawElements(GL_PATCHES, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*)0);
}