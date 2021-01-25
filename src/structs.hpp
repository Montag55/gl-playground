#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <variant>
#include <expansionMiddle.hpp>

const double DOUBLECLICK_TIME_MS = 333;

enum MouseButton {
	Left = 0,
	Middle = 1,
	Right = 2
};

enum MouseState {
    Click = 0,
    Drag = 1,
    Release = 2, 
    Double_Click = 3,
    Default = 4
};

struct Vertex {
    float id;               // should be uint
    float attIndx;          // should be uint
};

struct MouseStatus {
    bool buttons[3];
    glm::dvec2 pos;
    MouseState state;
    
    // start of left middle right mouse action
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

class ExpansionMiddle;
struct TimeExpansion {
    int leftAxisIndex; // left '3D' expansion axis ID
    int rightAxisIndex; // right '3D' expansion axis ID
    float angle; // tilt angle to fake '3D'
    
    std::vector<int> middleAxisIndicies; // axis from normal view, which are added to the middle
    
    glm::mat4 model_left;
    glm::mat4 model_right;
    glm::mat4 view;
    
    ExpansionMiddle* middle; // middle section for this entry
};
