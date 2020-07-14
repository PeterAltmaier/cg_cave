#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
    vec3 interp_normal;
	vec3 pos;
	vec3 toCameraVector;
} fs_in;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform sampler2D tex;

uniform float rand_light;

uniform vec3 light_dir;
uniform vec3 lightPos;
uniform vec3 viewPos;

const float pi = 3.14159265359;

vec2 getUVCoordinates(vec3 p);

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{

	float dist = 1.f/float(length(vec3(200.f,50.f,200.f)-fs_in.pos));

    vec3 color = texture(diffuseTexture, getUVCoordinates(normalize(fs_in.pos))).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;
    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color * dist *30* rand_light;

    FragColor = vec4(lighting, 1.0);
}

// void main()
// {
// 	vec2 tex_pos = vec2(pos.x,pos.z);

// 	vec3 light_dir_point = normalize(light_dir-normalize(fs_in.pos));
// 	float dist = 1.f/float(length(vec3(200.f,50.f,200.f)-pos));
// 	float light = dot(interp_normal,light_dir_point);//*dist*100*rand_light;
// 	vec3 light_colored = vec3(1.f,0.6f,0.2f)*light;
// 	vec2 tc = getUVCoordinates(normalize(pos));

// 	vec3 viewVector = normalize(toCameraVector);
// 	float refractiveFactor = dot(viewVector, interp_normal);

// 	FragColor = texture2D(tex,tc);
// 	FragColor =refractiveFactor* clamp(light, 0.1f, 1.f) * FragColor;
// 	//FragColor = vec4(light_colored,1.f)*FragColor;
// }

vec2 getUVCoordinates(vec3 p){
	vec2 uv = vec2(0.0,0.0);
	float r = sqrt(pow(p.x,2)+pow(p.y,2));
	float theta = atan(p.x,p.y)+pi;
	uv.x = theta/(2*pi);
	uv.y = (p.z+1)/2;

	//normalize(uv);

	return uv;
}