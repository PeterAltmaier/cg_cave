#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float vertices[200*200*6];

uniform int growth_fac;

out vec3 interp_normal;

struct face {
    vec3 vertices[3];
    vec3 normal;
    float space;
};

vec3 calcNormal(vec3 pos);

void main()
{
	gl_Position = projection * view * model * vec4(aPos.x, (growth_fac/30000.f)*aPos.y, aPos.z, 1.0);
	interp_normal = normalize((transpose(inverse(model)) * vec4(calcNormal(aPos), 0.0)).xyz);
}

vec3 calcNormal(vec3 pos){
	face faces[6];
	float total_space = 0;
	int index = 0;
	int face_index;
	vec3 normal_ret = vec3(0.f);
	//vertices am rand erstmal ignorieren
	if(pos.x == 0 || pos.z ==0 || pos.x == 199 || pos.z == 199)
		return vec3(0.f, 1.f, 0.f);

	//dreiecke um den Punkt herum berechnen
	for(float x = pos.x -1; x < pos.x +1; x++){
		for(float z = pos.z -1; z < pos.z+1; z++){
			//nur das "untere" Dreieck der Vertex
			if(z == pos.z-1 && x == pos.x){

				faces[index].vertices[0] = vec3(pos.x, (growth_fac/30000.f)*vertices[int((z * 200 + x) * 6 + 1)], pos.z);
				faces[index].vertices[1] = vec3(pos.x, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x) * 6 + 1)], pos.z + 1);
				faces[index].vertices[2] = vec3(pos.x + 1, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x + 1) * 6 + 1)], pos.z + 1);
				faces[index].normal = cross(faces[index].vertices[1] - faces[index].vertices[0], faces[index].vertices[2] - faces[index].vertices[0]);
				faces[index].space = 0.5f * length(faces[index].normal);
				faces[index].normal = normalize(faces[index].normal);

				total_space += faces[index].space;
				normal_ret += faces[index].normal;

				index++;
			}
			//nur das "obere" Dreieck der Vertex
			else if(z == pos.z && x == pos.x-1){

				faces[index].vertices[0] = vec3(pos.x, (growth_fac/30000.f)*vertices[int((z * 200 + x) * 6 + 1)], pos.z);
				faces[index].vertices[1] = vec3(pos.x +1, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x) * 6 + 1)], pos.z );
				faces[index].vertices[2] = vec3(pos.x + 1, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x + 1) * 6 + 1)], pos.z + 1);
				faces[index].normal = cross(faces[index].vertices[1] - faces[index].vertices[0], faces[index].vertices[2] - faces[index].vertices[0]);
				faces[index].space = 0.5f * length(faces[index].normal);
				faces[index].normal = normalize(faces[index].normal);

				total_space += faces[index].space;
				normal_ret += faces[index].normal;

				index++;
			}
			//beide Dreiecke
			else{
				//untere Dreieck
				faces[index].vertices[0] = vec3(pos.x, (growth_fac/30000.f)*vertices[int((z * 200 + x) * 6 + 1)], pos.z);
				faces[index].vertices[1] = vec3(pos.x, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x) * 6 + 1)], pos.z + 1);
				faces[index].vertices[2] = vec3(pos.x + 1, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x + 1) * 6 + 1)], pos.z + 1);
				faces[index].normal = cross(faces[index].vertices[1] - faces[index].vertices[0], faces[index].vertices[2] - faces[index].vertices[0]);
				faces[index].space = 0.5f * length(faces[index].normal);
				faces[index].normal = normalize(faces[index].normal);
				index++;

				total_space += faces[index].space;
				normal_ret += faces[index].normal;

				//obere Dreieck
				faces[index].vertices[0] = vec3(pos.x, (growth_fac/30000.f)*vertices[int((z * 200 + x) * 6 + 1)], pos.z);
				faces[index].vertices[1] = vec3(pos.x +1, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x) * 6 + 1)], pos.z );
				faces[index].vertices[2] = vec3(pos.x + 1, (growth_fac/30000.f)*vertices[int(((z + 1) * 200 + x + 1) * 6 + 1)], pos.z + 1);
				faces[index].normal = cross(faces[index].vertices[1] - faces[index].vertices[0], faces[index].vertices[2] - faces[index].vertices[0]);
				faces[index].space = 0.5f * length(faces[index].normal);
				faces[index].normal = normalize(faces[index].normal);

				index++;

				total_space += faces[index].space;
				normal_ret += faces[index].normal;
			}
		}

	}
	normal_ret = normal_ret / total_space;
	normal_ret = normalize(normal_ret);

	return normal_ret;
}

