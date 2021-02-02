#version 450 core

uniform mat4 transform;
uniform vec4 default_color;
uniform vec4 active_color;

layout(location = 0) in vec2 in_pos;
layout(location = 1) in float in_color_idx;

layout(location = 0) out vec4 pass_color;

float fadeEaseInOut(){
	return 0.0f;
}

void main() {
	gl_Position = transform * vec4(in_pos, 1.0, 1.0);
	
	// pass-through color
	if(in_color_idx == 0){
		pass_color = default_color;
	}
	else{
		pass_color = active_color;
	}
}
