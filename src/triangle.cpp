#include <application.hpp>

struct Vertex {
  glm::vec2 pos;
  glm::vec4 color;
};

class TriangleApp : public Application {
 public:
  TriangleApp() : Application{} {
    // setup shader program
    auto vert_shader = gl::load_shader_from_file("shaders/simple.vert");
    auto frag_shader = gl::load_shader_from_file("shaders/simple.frag");
    m_program = gl::create_program({vert_shader, frag_shader});

    // setup vertex array object and vertex buffer
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(Vertex));

    // setup position attribute
    GLuint pos_attrib_idx = 0;
    glEnableVertexArrayAttrib(m_vao, pos_attrib_idx);
    glVertexArrayAttribFormat(m_vao, pos_attrib_idx, 2, GL_FLOAT, false, offsetof(Vertex, pos));
    glVertexArrayAttribBinding(m_vao, pos_attrib_idx, 0);

    // setup color attribute
    GLuint color_attrib_idx = 1;
    glEnableVertexArrayAttrib(m_vao, color_attrib_idx);
    glVertexArrayAttribFormat(m_vao, color_attrib_idx, 4, GL_FLOAT, false, offsetof(Vertex, color));
    glVertexArrayAttribBinding(m_vao, color_attrib_idx, 0);

    // setup vertices
    std::vector<Vertex> vertices = {
        Vertex{glm::vec2{-1.0, -1.0}, glm::vec4{1.0, 0.0, 0.0, 1.0}},
        Vertex{glm::vec2{1.0, -1.0}, glm::vec4{0.0, 1.0, 0.0, 1.0}},
        Vertex{glm::vec2{0.0, 1.0}, glm::vec4{0.0, 0.0, 1.0, 1.0}},
    };
    glNamedBufferData(m_vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  }

  bool draw() const override {
    Application::draw();
    glUseProgram(m_program);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    return true;
  }

 protected:
  GLuint m_program;
  GLuint m_vao;
  GLuint m_vbo;
};

int main() {
  TriangleApp app;
  app.run();
  return 0;
}
