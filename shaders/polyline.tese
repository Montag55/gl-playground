#version 450 core

layout(isolines, equal_spacing, ccw) in;

patch in vec4 patch_color;
out vec4 tes_color;

// cubic Bezier
vec3 bezier(float u, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
	float B0 = (1.0 - u) * (1.0 - u) * (1.0 - u);
	float B1 = 3.0 * (1.0 - u) * (1.0 - u) * u;
	float B2 = 3.0 * (1.0 - u) * u * u;
	float B3 = u * u * u;
	return B0 * p0 + B1 * p1 + B2 * p2 + B3 * p3;
}

void main() {
	// start and end vertex
	vec3 p0 = vec3(gl_in[0].gl_Position);
	vec3 p3 = vec3(gl_in[1].gl_Position);

	// compute intermediate vertices here
	float intermediate_x = p0.x + 0.5 * (p3.x - p0.x);
	vec3 p1 = vec3(intermediate_x, p0.y, p0.z);
	vec3 p2 = vec3(intermediate_x, p3.y, p3.z);

	// compute the actual position on the Bezier line
	gl_Position = vec4(bezier(gl_TessCoord.x, p0, p1, p2, p3), 1.0);

	// pass-through color
	tes_color = patch_color;
}