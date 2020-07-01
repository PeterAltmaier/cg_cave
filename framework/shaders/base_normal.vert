#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int PLANE_WIDTH; 
uniform int PLANE_DEPTH;


uniform float growth_fac;
uniform float growth_range;
uniform sampler1D vertices_tex[10]; //todo anpassen falls groesse geandert

out vec3 interp_normal;

struct face {
    vec3 vertices[3];
    vec3 normal;
    float space;
};

vec3 calcNormal(vec3 pos);
float getY(float x, float z);

void main()
{
	gl_Position = projection * view * model * vec4(aPos.x, (growth_fac/growth_range)*aPos.y, aPos.z, 1.0);
	interp_normal = normalize((transpose(inverse(model)) * vec4(calcNormal(aPos), 0.0)).xyz);
}


vec3 calcNormal(vec3 pos){
	float growth_mult = growth_fac/growth_range;
	face face_temp;
	float total_space = 0;
	int face_index;
	vec3 normal_ret = vec3(0.f);
	//vertices am rand erstmal ignorieren, todo anpassen, falls größer
	if(pos.x == 0 || pos.z ==0 || pos.x == PLANE_DEPTH-1 || pos.z == PLANE_WIDTH-1)
		return vec3(0.f, 1.f, 0.f);

	//dreiecke um den Punkt herum berechnen
	for(float x = pos.x -1; x < pos.x +1; x++){
		for(float z = pos.z -1; z < pos.z+1; z++){
			//nur das "untere" Dreieck der Vertex
			if(z == pos.z-1 && x == pos.x){

				face_temp.vertices[0] = vec3(x, growth_mult * getY(x, z), z);
				face_temp.vertices[1] = vec3(x, growth_mult * getY(x, z+1),  z+ 1);
				face_temp.vertices[2] = vec3(x + 1, growth_mult * getY(x+1, z+1), z + 1);

				//face_temp.vertices[0] = vec3(x, (growth_fac/30000.f)*texture(vertices_tex, float((z * PLANE_DEPTH + x))/float(PLANE_DEPTH)).g, z);
				//face_temp.vertices[1] = vec3(x, (growth_fac/30000.f)*texture(vertices_tex, (float((z+1) *PLANE_DEPTH +x)/float(PLANE_DEPTH))).g,  z+ 1);
				//face_temp.vertices[2] = vec3(x + 1, (growth_fac/30000.f)*texture(vertices_tex, (float((z+1) *PLANE_DEPTH +x +1)/float(PLANE_DEPTH))).g, z + 1);
				face_temp.normal = cross(face_temp.vertices[1] - face_temp.vertices[0], face_temp.vertices[2] - face_temp.vertices[0]);
				face_temp.space = 0.5f * length(face_temp.normal);
				face_temp.normal = normalize(face_temp.normal);

				total_space += face_temp.space;
				normal_ret += face_temp.normal;

			}
			//nur das "obere" Dreieck der Vertex
			else if(z == pos.z && x == pos.x-1){
			
				face_temp.vertices[0] = vec3(x, growth_mult*getY(x,z), z);
				face_temp.vertices[1] = vec3(x+1,growth_mult*getY(x+1, z),  z);
				face_temp.vertices[2] = vec3(x + 1, growth_mult*getY(x+1, z+1), z + 1);

				//face_temp.vertices[0] = vec3(x, (growth_fac/30000.f)*texture(vertices_tex, float((z * PLANE_DEPTH + x))/float(PLANE_DEPTH)).g, z);
				//face_temp.vertices[1] = vec3(x+1, (growth_fac/30000.f)*texture(vertices_tex, (float((z) *PLANE_DEPTH +x+1)/float(PLANE_DEPTH))).g,  z);
				//face_temp.vertices[2] = vec3(x + 1, (growth_fac/30000.f)*texture(vertices_tex, (float((z+1) *PLANE_DEPTH +x +1)/float(PLANE_DEPTH))).g, z + 1);

				face_temp.normal = cross(face_temp.vertices[1] - face_temp.vertices[0], face_temp.vertices[2] - face_temp.vertices[0]);
				face_temp.space = 0.5f * length(face_temp.normal);
				face_temp.normal = normalize(face_temp.normal);

				total_space += face_temp.space;
				normal_ret += face_temp.normal;
				
			}
			//beide Dreiecke
			else{
				//untere Dreieck
				face_temp.vertices[0] = vec3(x, growth_mult * getY(x, z), z);
				face_temp.vertices[1] = vec3(x, growth_mult * getY(x, z+1),  z+ 1);
				face_temp.vertices[2] = vec3(x + 1, growth_mult * getY(x+1, z+1), z + 1);
				
				face_temp.normal = cross(face_temp.vertices[1] - face_temp.vertices[0], face_temp.vertices[2] - face_temp.vertices[0]);
				face_temp.space = 0.5f * length(face_temp.normal);
				face_temp.normal = normalize(face_temp.normal);

				total_space += face_temp.space;
				normal_ret += face_temp.normal;

				//obere Dreieck
				face_temp.vertices[0] = vec3(x, growth_mult*getY(x,z), z);
				face_temp.vertices[1] = vec3(x+1,growth_mult*getY(x+1, z),  z);
				face_temp.vertices[2] = vec3(x + 1, growth_mult*getY(x+1, z+1), z + 1);
				
				face_temp.normal = cross(face_temp.vertices[1] - face_temp.vertices[0], face_temp.vertices[2] - face_temp.vertices[0]);
				face_temp.space = 0.5f * length(face_temp.normal);
				face_temp.normal = normalize(face_temp.normal);

				total_space += face_temp.space;
				normal_ret += face_temp.normal;

				
			}
		}

	}
	normal_ret = normal_ret / total_space;
	normal_ret = normalize(normal_ret);

	return normal_ret;
}

float getY(float x, float z){
	float tex_coord = float(z * PLANE_DEPTH + x);
	if(tex_coord < 9000){
		return PLANE_DEPTH*texture(vertices_tex[0], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 18000){
		return PLANE_DEPTH*texture(vertices_tex[1], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 27000){
		return PLANE_DEPTH*texture(vertices_tex[2], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 36000){
		return PLANE_DEPTH*texture(vertices_tex[3], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 45000){
		return PLANE_DEPTH*texture(vertices_tex[4], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 54000){
		return PLANE_DEPTH*texture(vertices_tex[5], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 63000){
		return PLANE_DEPTH*texture(vertices_tex[6], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 72000){
		return PLANE_DEPTH*texture(vertices_tex[7], tex_coord/float(PLANE_DEPTH)).g;
	}
	else if(tex_coord < 81000){
		return PLANE_DEPTH*texture(vertices_tex[8], tex_coord/float(PLANE_DEPTH)).g;
	}
	else{
		return PLANE_DEPTH*texture(vertices_tex[9], tex_coord/float(PLANE_DEPTH)).g;
	}
}
