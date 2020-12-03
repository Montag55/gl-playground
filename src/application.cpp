#include <application.hpp>

Application::Application() : m_clear_color{0.0f}, m_mouse_pos{-1.0, -1.0} {
#ifdef NDEBUG
  spdlog::set_level(spdlog::level::info);
#else
  spdlog::set_level(spdlog::level::debug);
#endif

  if (!init_glfw()) {
    throw std::runtime_error("Failed to initialize GLFW!");
  }

  if (!create_window()) {
    throw std::runtime_error("Failed to create window!");
  }

  if (!init_opengl()) {
    throw std::runtime_error("Failed to initialize OpenGL!");
  }
}

Application::~Application() {
  glfwTerminate();
}

bool Application::should_close() const {
  return glfwWindowShouldClose(m_window);
}

const glm::vec2& Application::screen_size() const {
  return m_screen_size;
}

const glm::uvec2& Application::resolution() const {
  return m_resolution;
}

const glm::dvec2& Application::mouse_pos() const {
  return m_mouse_pos;
}

void Application::set_should_close(bool should_close) {
  glfwSetWindowShouldClose(m_window, should_close);
}

void Application::set_screen_size(const glm::vec2& screen_size) {
  m_screen_size = screen_size;
  spdlog::debug("Screen size was set to {}x{}", m_screen_size.x, m_screen_size.y);
}

void Application::set_mouse_pos(const glm::dvec2& mouse_pos) {
  glfwSetCursorPos(m_window, mouse_pos.x, m_resolution.y - mouse_pos.y);
  update_mouse_pos(mouse_pos.x, mouse_pos.y);
}

void Application::update_mouse_pos(double x, double y) {
  m_mouse_pos = glm::dvec2{x, y};
}

void Application::run() {
  while (!should_close()) {
    update();
    draw();
    glfwSwapBuffers(m_window);
  }
}

bool Application::update() {
  glfwPollEvents();
  return true;
}

bool Application::draw() const {
  glViewport(0, 0, m_resolution.x, m_resolution.y);
  glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  return true;
}

void Application::on_resize(int width, int height) {
  m_resolution = glm::uvec2{width, height};
  spdlog::debug("Window was resized to {}x{}", width, height);
}

void Application::on_key(int key, int scancode, int action, int mods) {
#ifndef NDEBUG
  if (action == GLFW_PRESS) {
    auto key_name = glfwGetKeyName(key, scancode);
    if (key_name) {
      spdlog::debug("Key '{}' was pressed", key_name);
    } else {
      spdlog::debug("Key {} was pressed", key);
    }
  }
#endif

  if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS)) {
    set_should_close(true);
  }
}

void Application::on_mouse_move(double dx, double dy) {}

void Application::on_mouse_button(int button, int action, int mods) {}

void Application::on_scroll(double x, double y) {}

bool Application::init_glfw() {
  if (!glfwInit()) {
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

  return true;
}

bool Application::create_window() {
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  auto monitor_name = std::string{glfwGetMonitorName(monitor)};
  spdlog::info("Using monitor '{}'", monitor_name);

  // create a fullscreen window on the selected monitor
  auto mode = glfwGetVideoMode(monitor);
  m_window = glfwCreateWindow(mode->width, mode->height, "gl-vis-playground", monitor, nullptr);

  // store the screen size
  int width_mm, height_mm;
  glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
  m_screen_size = glm::vec2{width_mm, height_mm} / 1000.0f;  // convert from mm to m

  // store the window dimensions
  m_resolution = glm::uvec2{mode->width, mode->height};

  if (!m_window) {
    spdlog::error("Failed to create GLFW window!");
    return false;
  }

  glfwMakeContextCurrent(m_window);

  // set this class as user pointer to access it in callbacks
  glfwSetWindowUserPointer(m_window, this);

  // error callback
  glfwSetErrorCallback([](int code, const char* description) {
    // simply log the error
    spdlog::error("GLFW Error {}: {}", code, description);
  });

  // framebuffer size callback
  glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->on_resize(width, height);
  });

  // keyboard callback
  glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->on_key(key, scancode, action, mods);
  });

  // mouse cursor pos callback
  glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double x, double y) {
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    const auto prev_pos = app->mouse_pos();
    const auto new_pos = glm::dvec2{x, app->resolution().y - y};
    double dx = 0.0;
    double dy = 0.0;
    if (prev_pos.x >= 0 && prev_pos.y >= 0) {
      dx = new_pos.x - prev_pos.x;
      dy = new_pos.y - prev_pos.y;
    }
    app->update_mouse_pos(new_pos.x, new_pos.y);
    app->on_mouse_move(dx, dy);
  });

  // mouse button callback
  glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->on_mouse_button(button, action, mods);
  });

  // scroll callback
  glfwSetScrollCallback(m_window, [](GLFWwindow* window, double x, double y) {
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->on_scroll(x, y);
  });

  return true;
}

bool Application::init_opengl() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    return false;
  }

#ifndef NDEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(debugMessageCallback, 0);
#endif

  return true;
}

void GLAPIENTRY debugMessageCallback(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar* message,
                                     const void* userParam) {
  // map severity to logging level
  auto level = spdlog::level::debug;
  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      level = spdlog::level::err;
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      level = spdlog::level::warn;
      break;
  }

  // map source to readable string
  std::string source_str = "api";
  switch (source) {
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      source_str = "window system";
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      source_str = "shader compiler";
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      source_str = "third party";
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      source_str = "application";
      break;
  }

  // map type to readable string
  std::string type_str = "other";
  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      type_str = "error";
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      type_str = "deprecated behaviour";
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      type_str = "undefined behaviour";
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      type_str = "portability";
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      type_str = "performance";
      break;
    case GL_DEBUG_TYPE_MARKER:
      type_str = "marker";
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      type_str = "push group";
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      type_str = "pop group";
      break;
  }

  spdlog::log(level, "OpenGL debug message [{}, {}]: {}", source_str, type_str, message);

  if (type == GL_DEBUG_TYPE_ERROR) {
    throw std::runtime_error("OpenGL Error: " + std::string(message));
  }
}
