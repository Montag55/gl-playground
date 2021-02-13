#version 450 core

uniform mat4 transform;
uniform int num_attributes;
uniform int num_data;
uniform int num_time;
uniform vec2 to_range;

layout(location = 0) in float in_id;
layout(location = 1) in float in_attribute;
layout(location = 2) in float in_time;

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

layout(std430, binding = 5) buffer timeBuffer {
	float times[];
};


float remap(float value, vec2 from, vec2 to) {
	return to.x + (value - from.x) * (to.y - to.x) / (from.y - from.x);
}

// cuic bezier
vec3 bezier(float u, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
	float B0 = (1.0 - u) * (1.0 - u) * (1.0 - u);
	float B1 = 3.0 * (1.0 - u) * (1.0 - u) * u;
	float B2 = 3.0 * (1.0 - u) * u * u;
	float B3 = u * u * u;
	return B0 * p0 + B1 * p1 + B2 * p2 + B3 * p3;
}

void main() {
	// get indecied and times
	float time = times[int(in_time)];
	float before = time;
	float after = clamp(before + 1, 0, num_time);
	
	// scale to fake perspective
	float before_scale = remap(1 - int(before) / float(num_time - 1), vec2(0, 1), vec2(0.125, 1));
	float after_scale = remap(1 - int(after) / float(num_time - 1), vec2(0, 1), vec2(0.125, 1));

	
	// get index for actual value
	int before_data_idx = int(num_attributes) * int(in_id) + int(in_attribute) + num_data * int(clamp(before, 0, num_time - 1));
	int after_data_idx = int(num_attributes) * int(in_id) + int(in_attribute) + num_data * int(clamp(after, 0, num_time - 1));
	
	// scale so normed range
	float p0_y = remap(values[before_data_idx], ranges[int(in_attribute)], vec2(-before_scale, before_scale));
	float p3_y = remap(values[after_data_idx], ranges[int(in_attribute)], vec2(-after_scale, after_scale));
	
	// create point
	vec3 p0 = vec3(vec2(attribute_coords[int(in_attribute)], p0_y), 0.0);
	vec3 p3 = vec3(vec2(attribute_coords[int(in_attribute)], p3_y), 0.0);

	// compute intermediate vertices here
	float intermediate_x = p0.x + 0.5 * (p3.x - p0.x);
	vec3 p1 = vec3(intermediate_x, p0.y, p0.z);
	vec3 p2 = vec3(intermediate_x, p3.y, p3.z);
	
	// get intermediat scaling
	float delta_y = before_scale - after_scale;
	float intermidiat_delta_y = delta_y * 0.5;

	float p1_scaled_y = remap(p1.y, vec2(-before_scale, before_scale), vec2(-before_scale + intermidiat_delta_y, before_scale - intermidiat_delta_y));
	float p2_scaled_y = remap(p2.y, vec2(-after_scale, after_scale), vec2(-after_scale - intermidiat_delta_y, after_scale + intermidiat_delta_y));

	gl_Position = transform * vec4(bezier(	before - int(before), 
											p0, 
											vec3(p1.x, p1_scaled_y, p1.z), 
											vec3(p2.x, p2_scaled_y, p2.z), 
											p3), 1.0);
	


	// gl_Position = vec4(before, after_scale, 0, 0);

	// pass-through color
	vs_color = colors[int(in_id)];
}
