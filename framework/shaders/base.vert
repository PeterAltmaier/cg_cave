#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 interp_normal;
	vec3 pos;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
    vec4 worldPositon = model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	gl_Position = projection * view * worldPositon;
	vs_out.interp_normal = normalize((transpose(inverse(model)) * vec4(aNormal, 0.0)).xyz);
	vs_out.pos = aPos;
}