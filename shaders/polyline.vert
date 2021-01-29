#version 450 core

uniform mat4 transform;
uniform int num_attributes;
uniform vec2 to_range;

layout(location = 0) in float in_id;
layout(location = 1) in float in_attribute;
layout(location = 2) in float in_axis;

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

layout(std430, binding = 3) buffer attributeBuffer {
	float attribute_coords[];
};


float remap(float value, vec2 from, vec2 to) {
	return to.x + (value - from.x) * (to.y - to.x) / (from.y - from.x);
}

void main() {
	int _dataIndex = int(num_attributes) * int(in_id) + int(in_attribute);
	float _value = values[_dataIndex];
	float _norm = remap(_value, ranges[int(in_attribute)], to_range);
	
	gl_Position = transform * vec4(vec2(attribute_coords[int(in_axis)], _norm), 0.0, 1.0);
	// gl_Position.z = _dataIndex;
	
	// pass-through color
	vs_color = colors[int(in_id)];
}
