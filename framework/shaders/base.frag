#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 interp_normal;
	vec3 pos;
} fs_in;


uniform sampler2D tex;

uniform float rand_light;

uniform vec3 light_pos_point;
uniform vec3 cameraPosition;
uniform float inner_radius;
uniform float outer_radius;
uniform vec3 cam_dir;
uniform int spotlight_activ;


const float pi = 3.14159265359;

vec2 getUVCoordinates(vec3 p);



void main()
{

	float dist = 1.f/float(length(vec3(200.f,50.f,200.f)-fs_in.FragPos));
    float dist_flash = 1.f/float(length(cameraPosition-fs_in.FragPos));
    //ermittelt die Texturdaten der Vertices
    vec3 color = texture(tex, getUVCoordinates(normalize(fs_in.pos))).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambienter Anteil
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(light_pos_point - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(cameraPosition - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;

    //"Helmlicht" im inneren Radius wird mit 1 skaliert und zwischen outer und inner Radius absteigend mit zunehmendem Abstand
    vec3 light_dir_flash = normalize(cameraPosition - fs_in.FragPos);
    float theta = dot(light_dir_flash, normalize(vec3(-1.f,-1.f,-1.f) * cam_dir));
    float epsi = inner_radius - outer_radius;
    //Skalierungsfaktor
    float smoothness = clamp((theta-outer_radius)/epsi,0.f,1.f);

    //Lichtstärke an dem betrachteten Punkt
    float spotlight = dot(fs_in.interp_normal, light_dir_flash) * smoothness * spotlight_activ;


    //vec3 lighting = (ambient + (diffuse + specular)) * color * dist * 30 * rand_light;
    vec3 lighting = ((ambient + diffuse + specular) * dist * 30 * rand_light * color + spotlight * 400 * pow(dist_flash,1.5) * vec3(0.7f,0.7f,.9f));//;
    //vec3 lighting = ( spotlight * 1 ) * color;
    FragColor = vec4(lighting, 1.0);
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