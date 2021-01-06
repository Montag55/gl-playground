#version 450 core
uniform vec4 color;

layout(location = 0) in vec2 in_pos;

layout(location = 0) out vec4 vs_color;

void main() {
	
	gl_Position = vec4(in_pos, 0.0, 1.0);
	
	// pass-through color
	vs_color = color;
}
