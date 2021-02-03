#version 450 core

layout(isolines, equal_spacing, ccw) in;

patch in vec4 patch_color;
patch in float patch_from_range;
patch in float patch_to_range;
patch in mat4 patch_model;

out vec4 tes_color;

// cubic Bezier
vec3 bezier(float u, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
	float B0 = (1.0 - u) * (1.0 - u) * (1.0 - u);
	float B1 = 3.0 * (1.0 - u) * (1.0 - u) * u;
	float B2 = 3.0 * (1.0 - u) * u * u;
	float B3 = u * u * u;
	return B0 * p0 + B1 * p1 + B2 * p2 + B3 * p3;
}

float remap(float value, vec2 from, vec2 to) {
	return to.x + (value - from.x) * (to.y - to.x) / (from.y - from.x);
}


void main() {
	// start and end vertex
	vec3 p0 = vec3(gl_in[0].gl_Position);
	vec3 p3 = vec3(gl_in[1].gl_Position);

	// compute intermediate vertices here
	float intermediate_x = p0.x + 0.5 * (p3.x - p0.x);
	vec3 p1 = vec3(intermediate_x, p0.y, p0.z);
	vec3 p2 = vec3(intermediate_x, p3.y, p3.z);
	
	/** 
	 *	determin needed scales:
	 *  p0 = 0 * delta_y (no offset)
	 *  p1 = 0.5 * delta_y (half way)
	 *  p2 = 0.5 * delta_y (half way)
	 *  p3 = 1 * delta_y (full offset)
	**/
	float delta_y = patch_from_range - patch_to_range;
	float intermidiat_delta_y = delta_y * 0.5;
	
	float p0_actual_y = remap(p0.y, vec2(-1,1), vec2(-patch_from_range, patch_from_range));
	float p1_actual_y = remap(p1.y, vec2(-1,1), vec2(-patch_from_range, patch_from_range));
	float p2_actual_y = remap(p2.y, vec2(-1,1), vec2(-patch_from_range, patch_from_range));
	float p3_actual_y = remap(p3.y, vec2(-1,1), vec2(-patch_to_range, patch_to_range));

	float p1_scaled_y = remap(p1_actual_y, vec2(-patch_from_range, patch_from_range), vec2(-patch_to_range - intermidiat_delta_y, patch_to_range + intermidiat_delta_y));
	float p2_scaled_y = remap(p2_actual_y, vec2(-patch_from_range, patch_from_range), vec2(-patch_to_range - intermidiat_delta_y, patch_to_range + intermidiat_delta_y));

	gl_Position = patch_model * vec4(bezier( gl_TessCoord.x, 
											 vec3(p0.x, p0_actual_y, p0.z), 
											 vec3(p1.x, p1_scaled_y, p1.z), 
											 vec3(p2.x, p2_scaled_y, p2.z), 
											 vec3(p3.x, p3_actual_y, p3.z)), 1.0);
	
	// pass-through color
	tes_color = patch_color;
}