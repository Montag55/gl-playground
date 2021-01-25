#version 450 core

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

uniform int num_data;
uniform int num_attrib;
uniform int attribute_idx;
uniform vec2 to_range;

layout(location = 0) in float in_id;
layout(location = 1) in float in_time;

layout(location = 0) out vec4 vs_color;

layout(std430, binding = 0) buffer dataBuffer {
	float values[];
};

layout(std430, binding = 1) buffer colorBuffer {
	vec4 colors[];
};

layout(std430, binding = 2) buffer rangeBuffer {
	vec2 ranges[];
};

layout(std430, binding = 3) buffer timeBuffer {
	float time_coords[];
};

float remap(float value, vec2 from, vec2 to) {
	return to.x + (value - from.x) * (to.y - to.x) / (from.y - from.x);
}

void main() {
	// determin depth scaling
	// float depth_val = remap((1 - in_time / num_attrib), vec2(0, 1), vec2(0.125, 1));
	// vec2 depth_range = vec2(-depth_val, depth_val);

	// get data value
	int _dataIndex = int(num_data) * int(in_time) + int(in_id) * num_attrib + attribute_idx;
	float _value = values[_dataIndex];
	
	// norm value according to scale
	float _norm = remap(_value, ranges[attribute_idx], to_range);
	
	gl_Position = projection * view * transform * vec4(time_coords[int(in_time)], _norm, 0, 1.0);

	// pass-through color
	vs_color = colors[int(in_id)];
}
