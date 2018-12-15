#version 330 core
uniform int alduinNum;
out vec4 color;
in vec3 fv_normal;
in vec2 fv_tex;
in vec3 fv_pos;
uniform vec3 campos;

uniform sampler2D tex;

void main()
{
	vec3 n = normalize(fv_normal);
	vec3 lp = vec3(10,-20,-100);
	vec3 ld = normalize(fv_pos - lp);
	float diffuse = dot(n,ld);

	color.rgb = texture(tex, fv_tex).rgb;

	color *= diffuse;

	vec3 cd = normalize(fv_pos - campos);
	vec3 h = normalize(cd+ld);
	float spec = dot(n,h);
	spec = clamp(spec,0,1);
	spec = pow(spec, 2);

	color.rgb += vec3(1,1,1) * spec;
	if (alduinNum == 0)
	{	
		color.a = 0.8;
	}
	else
	{
		color.a = 1.0;
	}

}
