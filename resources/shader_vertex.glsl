#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNorF;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec3 vertNorS;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 vertex_normal_flat;
out vec3 vertex_normal_smooth;
out vec2 vertex_tex;
out vec3 vertex_pos;
void main()
{
	vertex_normal_flat = vec4(M * vec4(vertNorF,0.0)).xyz;
	vertex_normal_smooth = vec4(M * vec4(vertNorS,0.0)).xyz;

	vertex_pos = vertPos;
	vec4 tpos =  M * vec4(vertPos, 1.0);
	gl_Position = P * V * tpos;
	vertex_tex = vertTex;
}
