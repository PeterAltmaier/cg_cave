#version 330 core

in vec3 interp_normal;
in vec3 pos;

uniform vec3 light_dir;
uniform sampler2D tex;

out vec4 FragColor;

const float pi = 3.14159265359;

vec2 getUVCoordinates(vec3 p);

void main()
{
	vec3 light_dir_point = normalize(light_dir-normalize(pos));
	float dist = 1.f/float(length(vec3(200.f,50.f,200.f)-pos));
	float light = dot(interp_normal,light_dir_point);
	vec2 tc = getUVCoordinates(normalize(pos));
	FragColor = texture2D(tex,tc);
	FragColor = clamp(light*dist*10.f, dist, 1.f) * FragColor;
}

vec2 getUVCoordinates(vec3 p){
	vec2 uv = vec2(0.0,0.0);
	float r = sqrt(pow(p.x,2)+pow(p.y,2));
	float theta = atan(p.x,p.y)+pi;
	uv.x = theta/(2*pi);
	uv.y = (p.z+1)/2;
	//uv = uv/sqrt(pow(uv.x,2)+pow(uv.y,2));
	normalize(uv);

	return uv;
}