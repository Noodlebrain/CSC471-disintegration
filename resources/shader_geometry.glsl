#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vertex_normal_flat[3];
in vec3 vertex_normal_smooth[3];
in vec2 vertex_tex[3];
in vec3 vertex_pos[3];

out vec3 fv_normal;
out vec2 fv_tex;
out vec3 fv_pos;

uniform float time;
uniform int alduinNum;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normalize(normal) * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
}

vec4 avg_pos(vec4 posA, vec4 posB, vec4 posC)
{
	float avgX = (posA.x + posB.x + posC.x) / 3.0;
	float avgY = (posA.y + posB.y + posC.y) / 3.0;
	float avgZ = (posA.z + posB.z + posC.z) / 3.0;
	return vec4(avgX, avgY, avgZ, 0.0);
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

vec4 decay(vec4 rotPos, vec4 rotPosA, vec4 rotPosB, vec3 origPos)
{
	float magnitude = 0.75;
	float delayed_time = max(0.0, 0.75 * time - (4 * (origPos.x + 1) * 2 * (origPos.y + 1)));
	if (delayed_time <= 0.0)
	{
		return rotPos;
	}
	vec3 direction = vec3(	rotPos.x * sin(delayed_time) * magnitude,
							pow(delayed_time, 1.5),
							rotPos.z * sin(delayed_time) * magnitude);
	vec4 tri_avg_pos = avg_pos(rotPos, rotPosA, rotPosB);
	mat4 rotate = rotationMatrix(vec3(-0.5, 0.0, 0.0), delayed_time / 2.0);
	return (min(1.0, 1.0 - pow(delayed_time / 2, 2)) * rotPos + min(1.0, pow(delayed_time / 2, 2))
		* tri_avg_pos + vec4(direction, 0.0)) * rotate;
}

void main()
{
	// gl_Position = explode(gl_in[0].gl_Position, vertex_normal_flat[0]);
	gl_Position = decay(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, vertex_pos[1]);
	if (alduinNum == 0)
	{
		fv_normal = vertex_normal_flat[0];
	}
	else
	{
		fv_normal = vertex_normal_smooth[0];
	}
	fv_tex = vertex_tex[0];
	fv_pos = gl_Position.xyz;
    EmitVertex();

    // gl_Position = explode(gl_in[1].gl_Position, vertex_normal_flat[1]);
	gl_Position = decay(gl_in[1].gl_Position, gl_in[0].gl_Position, gl_in[2].gl_Position, vertex_pos[1]);
	if (alduinNum == 0)
	{
		fv_normal = vertex_normal_flat[1];
	}
	else
	{
		fv_normal = vertex_normal_smooth[1];
	}
	fv_tex = vertex_tex[1];
	fv_pos = gl_Position.xyz;
    EmitVertex();

    // gl_Position = explode(gl_in[2].gl_Position, vertex_normal_flat[2]);
	gl_Position = decay(gl_in[2].gl_Position, gl_in[0].gl_Position, gl_in[1].gl_Position, vertex_pos[1]);
	if (alduinNum == 0)
	{
		fv_normal = vertex_normal_flat[2];
	}
	else
	{
		fv_normal = vertex_normal_smooth[2];
	}
	fv_tex = vertex_tex[2];
	fv_pos = gl_Position.xyz;
    EmitVertex();
    EndPrimitive();
}
