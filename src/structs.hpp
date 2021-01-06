#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <glm/glm.hpp>
#include <variant>

enum MouseButton {
	Left = 0,
	Middle = 1,
	Right = 2
};

enum MouseState {
    Click = 0,
    Drag = 1,
    Release = 2,
    Default = 3
};

struct Vertex {
    float id;               // should be uint
    float attIndx;          // should be uint
};

struct MouseStatus {
    bool buttons[3];
    glm::dvec2 pos;
    MouseState state;
    std::chrono::time_point<std::chrono::system_clock> time[3];
    float distance;
};

struct Point {
    // now no point in struct, clean up later
    glm::vec2 pos;
};

struct AABB {
    glm::vec2 c1;
    glm::vec2 c2;
};

struct AxisVertex {
    glm::vec2 pos;
    float colorIndx;
};

struct SortObj {
    float val;
    int index;
};