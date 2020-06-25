#version 330 core

in vec3 interp_normal;

uniform vec3 light_dir;
uniform sampler2D tex;

out vec4 FragColor;

const float pi = 3.14159265359;

vec2 getUVCoordinates(vec3 p);

void main()
{
	float light = dot(interp_normal, light_dir);
	vec2 tc = getUVCoordinates(normalize(interp_normal));
	FragColor = texture2D(tex,tc);
	FragColor = clamp(light, 0.1, 1.0) * FragColor;
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