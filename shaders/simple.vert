#version 450 core

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec4 in_color;
layout(location = 0) out vec4 v_color;

void main() {
  gl_Position = in_pos;

  // pass-through color
  v_color = in_color;
}
