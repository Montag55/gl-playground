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

add_executable( graph
    external/glad/src/glad.c
    src/application.cpp
    src/gl/program.cpp
    src/gl/shader.cpp
    src/boxSelect.cpp
    src/axisDrag.cpp
    src/tool.cpp
    src/timeSeries.cpp
    src/expansionMiddle.cpp
    src/expansionActive.cpp
    src/graphApp.cpp  )
target_link_libraries(graph ${LIBRARIES})

file(GLOB_RECURSE SHADERFILES  ${CMAKE_BINARY_DIR}/shaders/*)
list(LENGTH SHADERFILES RES_LEN) 

if(RES_LEN GREATER 0)
    message("[Custom] Cleaning up old shader data")
    file (REMOVE ${SHADERFILES})
endif()

# copy shaders
message("[Custom] Copying shader data")
file(COPY shaders DESTINATION .)
