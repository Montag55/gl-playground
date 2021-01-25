#version 450 core

uniform mat4 transform;
uniform vec4 active_color;
uniform vec4 thickness_offset;

layout(location = 0) in float in_attribute;
layout(location = 1) in float in_y;

layout(location = 0) out vec4 vs_color;

layout(std430, binding = 3) buffer attributeBuffer {
	float attribute_coords[];
};

void main() {
	vec4 scaled_offset = thickness_offset / 2; 
	gl_Position = transform * vec4(vec2(attribute_coords[int(in_attribute)] + scaled_offset[gl_VertexID], in_y), 0.0, 1.0);

	// pass-through color
	vs_color = active_color;
}
