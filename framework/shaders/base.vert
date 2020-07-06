#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPosition;

out vec3 interp_normal;
out vec3 pos;
out vec3 toCameraVector;

void main()
{
	vec4 worldPositon = model * vec4(aPos.x, aPos.y, aPos.z, 1.0); 
	gl_Position = projection * view * worldPositon; 
	interp_normal = normalize((transpose(inverse(model)) * vec4(normal, 0.0)).xyz);
	pos = aPos;
	toCameraVector = cameraPosition - worldPositon.xyz;
}


