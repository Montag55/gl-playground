cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(gl-vis-playground)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# external libraries
include_directories(external/spdlog/include)
include_directories(external/glad/include)
include_directories(external/glm)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
add_subdirectory(external/glfw)
include_directories(external/glfw/include)

include_directories(src)

set(LIBRARIES ${OPENGL_LIBRARIES} glfw)

add_executable(triangle
    external/glad/src/glad.c
    src/application.cpp
    src/gl/program.cpp
    src/gl/shader.cpp
    src/triangle.cpp
)
target_link_libraries(triangle ${LIBRARIES})

# copy shaders
file(COPY shaders DESTINATION shaders)