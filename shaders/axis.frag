#version 450 core

layout(location = 0) in vec4 pass_color;

out vec4 out_color;

void main() {
    out_color = pass_color;
}