#include <application.hpp>
#include <utils.hpp>

struct Vertex {
    float id;       // should be uint
    float attIndx;  // should be uint
    float axis_pos;
    glm::vec2 from_range;
    glm::vec2 to_range;
};

class GraphApp : public Application {
 public:
    GraphApp() : Application{} {
        // setup shader program
        auto vert_shader = gl::load_shader_from_file("shaders/graph.vert");
        auto frag_shader = gl::load_shader_from_file("shaders/graph.frag");
        m_program = gl::create_program({vert_shader, frag_shader});
        
        initializeData();        
        initializeAxis(4);
        initializeVertexBuffers();
        initializeStorageBuffers();
      
        for (int i = 0; i < m_data.size() / m_axis.size(); i++) {
            m_first.push_back(i * m_axis.size());
            m_count.push_back(m_axis.size());
        }

    }

  ~GraphApp() {
    if (glIsBuffer(m_vbo)) {
      glDeleteBuffers(1, &m_vbo);
    }
    if (glIsVertexArray(m_vao)) {
      glDeleteVertexArrays(1, &m_vao);
    }
    if (glIsBuffer(m_data_ssbo)) {
      glDeleteBuffers(1, &m_data_ssbo);
    }
    if (glIsProgram(m_program)) {
      glDeleteProgram(m_program);
    }
  }

    bool draw() const override {
        Application::draw();
        glUseProgram(m_program);
        
        auto model = glm::scale(glm::mat4{1.0f}, glm::vec3{0.8f});
        gl::set_program_uniform(m_program, glGetUniformLocation(m_program, "transform"), model);
        glProgramUniform1i(m_program, glGetUniformLocation(m_program, "num_attributes"), m_axis.size());
       
        glBindVertexArray(m_vao);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_data_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_color_ssbo);
        glMultiDrawArrays(GL_LINE_STRIP, m_first.data(), m_count.data(), m_data.size() / m_axis.size());
               
        return true;
    }
    
    void initializeData() {
        Utils::readData(m_data, "../../iris.txt");
        Utils::getMaxValues(m_data, m_max);
        Utils::getMinValues(m_data, m_min);
        
        if (m_data.empty()) {
			throw std::runtime_error("Failedto initialze data!");
		}
    }
    
    void initializeAxis(const int& num_attributes) {
        for (int i = 0; i < num_attributes; i++) {
            m_axis.push_back(Utils::remap((float)i / (num_attributes - 1), glm::vec2{0,1}, glm::vec2{-1,1}));
        }

        // spdlog::info("axis [{}, {}, {}, {}]", m_axis[0], m_axis[1], m_axis[2], m_axis[3]);
    }

    void initializeVertexBuffers() {
        /**
        *   Only passing in floats since int and uint seems to have padding / offset errors.
        *   ID and AttribueIndex are sated to in in shader           
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

        // setup y-coord id attribute
        GLuint axis_pos_attrib_idx = 2;
        glEnableVertexArrayAttrib(m_vao, axis_pos_attrib_idx);
        glVertexArrayAttribFormat(m_vao, axis_pos_attrib_idx, 1, GL_FLOAT, false, offsetof(Vertex, axis_pos));
        glVertexArrayAttribBinding(m_vao, axis_pos_attrib_idx, 0);

        // setup from_range attribute
        GLuint from_range_attrib_idx = 3;
        glEnableVertexArrayAttrib(m_vao, from_range_attrib_idx);
        glVertexArrayAttribFormat(m_vao, from_range_attrib_idx, 2, GL_FLOAT, false, offsetof(Vertex, from_range));
        glVertexArrayAttribBinding(m_vao, from_range_attrib_idx, 0);

        // setup to_range attribute
        GLuint to_range_attrib_idx = 4;
        glEnableVertexArrayAttrib(m_vao, to_range_attrib_idx);
        glVertexArrayAttribFormat(m_vao, to_range_attrib_idx, 2, GL_FLOAT, false, offsetof(Vertex, to_range));
        glVertexArrayAttribBinding(m_vao, to_range_attrib_idx, 0);

        // setup vertices
        std::vector<Vertex> vertices;
        for (int i = 0; i < m_data.size(); i++) {
            vertices.push_back( Vertex{
                float(i / m_axis.size()),
                float(i % m_axis.size()),
                float(m_axis[i % m_axis.size()]),
                glm::vec2{m_min[i % m_axis.size()], m_max[i % m_axis.size()]},
                glm::vec2{-1, 1}
            });
        }
        
        glNamedBufferData(m_vbo, Utils::vectorsizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    }
    
    void initializeStorageBuffers() {         
        GLuint data_binding = 0;
		glCreateBuffers(1, &m_data_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_binding, m_data_ssbo);
		glNamedBufferData(m_data_ssbo, Utils::vectorsizeof(m_data), m_data.data(), GL_DYNAMIC_DRAW);
        
        // color data
        for (int i = 0; i < m_data.size() / m_axis.size(); i++) {
            m_colors.push_back(glm::vec4{1,0,0,1});
            m_colors.push_back(glm::vec4{0,1,0,1});
            m_colors.push_back(glm::vec4{0,0,1,1});
            m_colors.push_back(glm::vec4{1,1,1,1});
        }

        GLuint color_binding = 1;
		glCreateBuffers(1, &m_color_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, color_binding, m_color_ssbo);
		glNamedBufferData(m_color_ssbo, Utils::vectorsizeof(m_colors), m_colors.data(), GL_DYNAMIC_DRAW);
	}

 protected:
    GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_data_ssbo;
    GLuint m_color_ssbo;

    std::vector<float> m_data;
    std::vector<glm::vec4> m_colors;
    std::vector<int> m_count;
    std::vector<int> m_first;
    glm::vec4 m_max;
    glm::vec4 m_min;
    std::vector<float> m_axis;
};

int main() {
    GraphApp app; 
    app.run();
    return 0;
}
