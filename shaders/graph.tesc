#version 450 core

layout(vertices = 2) out;

layout(location = 0) in vec4 vs_color[];
patch out vec4 patch_color;

void main() {
	// compute the distance in pixels between the two vertices
	vec2 start = gl_in[0].gl_Position.xy;
	vec2 end = gl_in[1].gl_Position.xy;
	float dist = length(vec2(0.5 * (end - start)));

	// set tesselation levels
	gl_TessLevelOuter[0] = 1;
	gl_TessLevelOuter[1] = 64;
	// gl_TessLevelOuter[1] = max(1, floor(dist / 20.0) * 0.5);

	// pass through vertex positions
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
	
	// pass-through color
	patch_color = vs_color[gl_InvocationID + 1];
}