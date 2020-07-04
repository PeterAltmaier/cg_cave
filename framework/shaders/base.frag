#version 330 core

in vec3 interp_normal;
in vec3 pos;

uniform vec3 light_dir;
uniform sampler2D tex;
uniform float rand_light;

out vec4 FragColor;

const float pi = 3.14159265359;

vec2 getUVCoordinates(vec3 p);

void main()
{
	vec2 tex_pos = vec2(pos.x,pos.z);

	vec3 light_dir_point = normalize(light_dir-normalize(pos));
	float dist = 1.f/float(length(vec3(200.f,50.f,200.f)-pos));
	float light = dot(interp_normal,light_dir_point)*dist*30*rand_light;
	vec3 light_colored = vec3(1.f,0.6f,0.2f)*light;
	vec2 tc = getUVCoordinates(normalize(pos));
	FragColor = texture2D(tex,tc);
	FragColor = clamp(light, 0.01f, 1.f) * FragColor;
	//FragColor = vec4(light_colored,1.f)*FragColor;
}

vec2 getUVCoordinates(vec3 p){
	vec2 uv = vec2(0.0,0.0);
	float r = sqrt(pow(p.x,2)+pow(p.y,2));
	float theta = atan(p.x,p.y)+pi;
	uv.x = theta/(2*pi);
	uv.y = (p.z+1)/2;

	//normalize(uv);

	return uv;
}