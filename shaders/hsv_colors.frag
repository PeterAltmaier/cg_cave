#version 330 core

in vec4 interp_color;

out vec4 frag_color;

vec4 hsva2rgba(vec4 hsva) {
    return hsva;
}

void main()
{
    frag_color = hsva2rgba(interp_color);
}
