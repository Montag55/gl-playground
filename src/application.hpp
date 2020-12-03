#pragma once

#include <memory>
#include <set>
#include <string>
#include <tuple>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <gl/program.hpp>
#include <gl/shader.hpp>

class Application {
 public:
  Application();
  Application(const Application&) = delete;
  Application(Application&&) = default;
  virtual ~Application();

  Application& operator=(const Application& other) = delete;
  Application& operator=(Application&& other) = default;

  // getters
  bool should_close() const;
  const glm::vec2& screen_size() const;
  const glm::uvec2& resolution() const;
  const glm::dvec2& mouse_pos() const;

  // setters
  void set_should_close(bool should_close);
  void set_screen_size(const glm::vec2& screen_size);
  void set_mouse_pos(const glm::dvec2& mouse_pos);

  void update_mouse_pos(double x, double y);

  virtual void run();
  virtual bool update();
  virtual bool draw() const;

  // glfw callbacks
  virtual void on_resize(int width, int height);
  virtual void on_key(int key, int scancode, int action, int mods);
  virtual void on_mouse_move(double dx, double dy);
  virtual void on_mouse_button(int button, int action, int mods);
  virtual void on_scroll(double x, double y);

 protected:
  bool init_glfw();
  bool create_window();
  bool init_opengl();
  void poll_touch_events();

 protected:
  // window
  GLFWwindow* m_window;
  glm::vec3 m_clear_color;
  glm::vec2 m_screen_size;
  glm::uvec2 m_resolution;
  glm::dvec2 m_mouse_pos;
};

void GLAPIENTRY debugMessageCallback(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar* message,
                                     const void* userParam);
