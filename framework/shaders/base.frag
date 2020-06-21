#version 330 core

in vec3 interp_normal;

uniform vec3 light_dir;

out vec4 FragColor;


void main()
{
	float light = dot(interp_normal, light_dir);
	FragColor = clamp(light, 0.1, 1.0) * vec4(0.0f, 0.6f, 0.0f, 1.0f);
}

