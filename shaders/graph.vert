#version 450 core

uniform mat4 transform;
uniform int num_attributes;


layout(location = 0) in float in_id;
layout(location = 1) in float in_attribute_index;
layout(location = 2) in float in_axis_pos;
layout(location = 3) in vec2 in_from_range;
layout(location = 4) in vec2 in_to_range;

layout(location = 0) out vec4 v_color;

layout(std430, binding = 0) buffer dataBuffer {
	float values[];
};

layout(std430, binding = 1) buffer colorBuffer {
	vec4 colors[];
};


float remap(float value, vec2 from, vec2 to) {
	return to.x + (value - from.x) * (to.y - to.x) / (from.y - from.x);
}

void main() {
	int _dataIndex = int(num_attributes) * int(in_id) + int(in_attribute_index);
	float _value = values[_dataIndex];
	float _norm = remap(_value, in_from_range, in_to_range);
	
	gl_Position = transform * vec4(vec2(in_axis_pos, _norm), 0.0, 1.0);
	
	// pass-through color
	v_color = colors[_dataIndex];
}
