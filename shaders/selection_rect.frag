#version 450 core

in vec4 vs_color;

out vec4 out_color;

void main() {
    out_color = vs_color;
}