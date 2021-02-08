#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <variant>
#include <expansionMiddle.hpp>
#include <expansionActive.hpp>
#include <expansionHandles.hpp>

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

enum Handle {
    LeftHandle = 0,
    RightHandle = 1
};

struct Vertex {
    float id;               // should be uint
    float attIndx;          // should be uint
    float timeIndx;         // should be unit
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

struct NonAABB {
    glm::vec2 uL;
    glm::vec2 uR;
    glm::vec2 lL;
    glm::vec2 lR;
};

struct SortObj {
    float val;
    int index;
};

class ExpansionMiddle;
class ExpansionActive;
class ExpansionHandles;
struct TimeExpansion {
    int leftAxisIndex; // left '3D' expansion axis ID
    int rightAxisIndex; // right '3D' expansion axis ID
    float angle; // tilt angle to fake '3D'
    
    std::vector<int> middleAxisIndicies; // axis from normal view, which are added to the middle
    
    glm::mat4 model_left;
    glm::mat4 model_right;
    glm::mat4 view;
    
    ExpansionMiddle* middle; // middle section for this entry
    ExpansionActive* addVisualizer; // highlighter when adding axis to expansion
    ExpansionHandles* left_handle; // left handles for this entry
    ExpansionHandles* right_handle; // left handles for this entry
};

