#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_dir;


out vec3 interp_normal;
out vec3 interp_light_dir;


void main()
{
	gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	interp_normal = normalize((transpose(inverse(model)) * vec4(normal, 0.0)).xyz);
	interp_light_dir = normalize((view * vec4(light_dir, 0.0)).xyz);
}


