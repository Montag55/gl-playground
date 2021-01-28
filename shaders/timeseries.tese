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
	vec3 halfway_point = (p0 + p3) / 2;
	float delta_half_height = abs(p0.y - p3.y) / 2;
		
	vec3 p1 = halfway_point;
	vec3 p2 = halfway_point;
	
	if(p0.y < p3.y){
		p1.y -= delta_half_height;
		p2.y += delta_half_height;
	}
	else{
		p1.y += delta_half_height;
		p2.y -= delta_half_height;
	}


	//gl_Position = vec4(bezier(gl_TessCoord.x, p0, p1, p2, p3), 1.0);
	vec4 pos = vec4(bezier(gl_TessCoord.x, p0, p1, p2, p3), 1.0);
	
	float scale_y = (patch_from_range - patch_to_range) - (patch_from_range - patch_to_range) * gl_TessCoord.x;
	
	float from_y = remap(pos.y, vec2(-1,1), vec2(-patch_from_range, patch_from_range));
	float to_y = remap(from_y, vec2(-patch_from_range, patch_from_range), vec2(-patch_to_range - scale_y, patch_to_range + scale_y));

	gl_Position = patch_model * vec4(pos.x, to_y, 0, 1.0);
	
	// pass-through color
	tes_color = patch_color;
}