#include <application.hpp>
#include <utils.hpp>

struct Vertex {
    float id;               // should be uint
    float attIndx;          // should be uint
    glm::vec2 from_range;   // should be done by instancing
    glm::vec2 to_range;     // should be done by instancing
};

struct DrawArrayCommand {
    GLuint vertexCount;
    GLuint instanceCount;
    GLuint firstIndex;
    GLuint baseInstance;
};

class GraphApp : public Application {
 public:
    GraphApp() : Application{} {
        // setup shader program
        auto vert_shader = gl::load_shader_from_file("shaders/graph.vert", GL_VERTEX_SHADER);
        auto tcs_shader = gl::load_shader_from_file("shaders/graph.tesc", GL_TESS_CONTROL_SHADER);
        auto tes_shader = gl::load_shader_from_file("shaders/graph.tese", GL_TESS_EVALUATION_SHADER);
        auto frag_shader = gl::load_shader_from_file("shaders/graph.frag", GL_FRAGMENT_SHADER);
        m_program = gl::create_program({vert_shader, tcs_shader, tes_shader, frag_shader});
        
        // init data
        initializeData();
        initializeAxis(4);
        initializeIndexBuffer();
        initializeColor();        

        // init gpu buffers
        initializeVertexBuffers();
        initializeStorageBuffers();
        
        // activate color blending and setup background color
        m_clear_color = glm::vec3(0.125, 0.133, 0.156);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
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
    if (glIsBuffer(m_color_ssbo)) {
      glDeleteBuffers(1, &m_color_ssbo);
    }
    if (glIsBuffer(m_ibo)) {
      glDeleteBuffers(1, &m_ibo);
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
        
        // bind buffers eventhough they were never unbinded, just to be sure
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo); 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_data_ssbo);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_color_ssbo);
        
        // tell tesellation shader how many verts per line
        glPatchParameteri(GL_PATCH_VERTICES, 2);
        
        // uses buffer currently bound to GL_ELEMENT_ARRAY_BUFFER
        glDrawElements(GL_PATCHES, m_indicies.size(), GL_UNSIGNED_SHORT, (const void*) 0);        
        
        return true;
    }
    
    void initializeData() {
        Utils::readData(m_data, "../../iris.txt");
        Utils::getMaxValues(m_data, m_max);
        Utils::getMinValues(m_data, m_min);
        
        if (m_data.empty()) {
			throw std::runtime_error("Failed to initialze data!");
		}
    }
    
    void initializeColor() {
        // has to be called after initializeData(), initializeAxis()
        if (m_data.empty() || m_axis.empty()) {
            throw std::runtime_error("Failed to initialze Color data!");
        }

        for (int i = 0; i < m_data.size() / m_axis.size(); i++) {
            m_colors.push_back(glm::vec4(0.321, 0.580, 0.886, 0.4f));
        }
    }

    void initializeAxis(const int& num_attributes) {
        for (int i = 0; i < num_attributes; i++) {
            m_axis.push_back(Utils::remap((float)i / (num_attributes - 1), glm::vec2{0,1}, glm::vec2{-1,1}));
        }
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

        // setup from_range attribute
        GLuint from_range_attrib_idx = 2;
        glEnableVertexArrayAttrib(m_vao, from_range_attrib_idx);
        glVertexArrayAttribFormat(m_vao, from_range_attrib_idx, 2, GL_FLOAT, false, offsetof(Vertex, from_range));
        glVertexArrayAttribBinding(m_vao, from_range_attrib_idx, 0);

        // setup to_range attribute
        GLuint to_range_attrib_idx = 3;
        glEnableVertexArrayAttrib(m_vao, to_range_attrib_idx);
        glVertexArrayAttribFormat(m_vao, to_range_attrib_idx, 2, GL_FLOAT, false, offsetof(Vertex, to_range));
        glVertexArrayAttribBinding(m_vao, to_range_attrib_idx, 0);

        // setup vertices
        std::vector<Vertex> vertices;
        for (int i = 0; i < m_data.size(); i++) {
            vertices.push_back( Vertex{
                float(i / m_axis.size()),
                float(i % m_axis.size()),
                glm::vec2{m_min[i % m_axis.size()], m_max[i % m_axis.size()]},
                glm::vec2{-1, 1}
            });
        }

        glNamedBufferData(m_vbo, Utils::vectorsizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    }
    
    void initializeStorageBuffers() {         
        // setup data ssbo
        GLuint data_binding = 0;
		glCreateBuffers(1, &m_data_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, data_binding, m_data_ssbo);
		glNamedBufferData(m_data_ssbo, Utils::vectorsizeof(m_data), m_data.data(), GL_DYNAMIC_DRAW);
             
        // setup color ssbo
        GLuint color_binding = 1;
		glCreateBuffers(1, &m_color_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, color_binding, m_color_ssbo);
		glNamedBufferData(m_color_ssbo, Utils::vectorsizeof(m_colors), m_colors.data(), GL_DYNAMIC_DRAW);
        
        // setup attribute axis pos (x-coord) ssbo
        GLuint attribute_pos_binding = 2;
		glCreateBuffers(1, &m_attribute_ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, attribute_pos_binding, m_attribute_ssbo);
		glNamedBufferData(m_attribute_ssbo, Utils::vectorsizeof(m_axis), m_axis.data(), GL_DYNAMIC_DRAW);
	}
    
    void initializeIndexBuffer() {
        // has to be called after initializeData(), initializeAxis()
        if (m_data.empty() || m_axis.empty()) {
            throw std::runtime_error("Failed to initialze Color data!");
        }
        
        // tes-schader can't do linestrips -> push all vertex indicies 
        // in twice except for first and last of each line 
        for (unsigned short i = 0; i < m_data.size(); i++) {
            m_indicies.push_back(i);
            if (i %  m_axis.size() != 0 && i % m_axis.size() != m_axis.size() - 1) {
                m_indicies.push_back(i);
            }
        }

        // Bind to Element array buffer -> Indexing so DrawElements can be used
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, Utils::vectorsizeof(m_indicies), m_indicies.data(), GL_STATIC_DRAW);
    }

 protected:
    GLuint m_program;
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_data_ssbo;
    GLuint m_color_ssbo;
    GLuint m_attribute_ssbo;

    glm::vec4 m_max;
    glm::vec4 m_min;
    std::vector<float> m_axis;
    std::vector<float> m_data;
    std::vector<glm::vec4> m_colors;
    std::vector<unsigned short> m_indicies;

};

int main() {
    GraphApp app; 
    app.run();
    return 0;
}
