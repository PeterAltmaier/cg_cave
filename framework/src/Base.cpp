#define _USE_MATH_DEFINES

#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include <camera.hpp>
#include "mesh.hpp"
#include <algorithm>
#include <iterator>
#include "HeightGenerator.h"
#include "stb_image.h"
#include <math.h>
#include <random>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
const float FOV = 90.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 1000.f;
const unsigned int PLANE_WIDTH = 400;
const unsigned int PLANE_DEPTH = 400;
const unsigned int NUM_STICKS = 800;
const unsigned int PLANE_DIFF = 115;
int spotlight = 0;

glm::mat4 proj_matrix;

struct face {
    glm::vec3 vertices[3];
    glm::vec3 normal;
    float space;
};

// called whenever the window gets resized
void resizeCallback(GLFWwindow* window, int width, int height);

void calculateNormals(float* vertices, unsigned int v_size, face* faces, unsigned int f_size, HeightGenerator generator,int invert);

void generateIndices(unsigned int *indices, unsigned int ind_size);

void processInput(GLFWwindow *window,float delta_time,float *momentum,float *period);

void assign_momentum(unsigned int *pressed,float *momentum,float *period);

void key_pressed(GLFWwindow *window,unsigned int *pressed);

void generateVertices(float *vertices, HeightGenerator generator);

void generateSticks(float* sticks_data,int** stick_col_mat);

void growth_plane(float* vertices, float* tmp_vertices, float growth_factor, float growth_range);

void growSticks(float* vertices, float* vertices_floor, float* sticks_data);

bool stick_col_set(int** col_mat, int x, int z, float radius);

float compAdd(glm::mat3 matrix);

glm::vec2 drop_derivation(float* vertices, float x, float z);

void generateDrops(float* vertices_ceil, float* vertices_floor);

int
main(int, char* argv[]) {

    //vertex data
    
    unsigned int num_vertices = PLANE_DEPTH * PLANE_WIDTH;
    float* vertices_floor = new float[PLANE_DEPTH * PLANE_WIDTH * 6];

    float* vertices_floor_tmp = new float[PLANE_DEPTH * PLANE_WIDTH * 6];
    float* vertices_ceil_tmp = new float[PLANE_DEPTH * PLANE_WIDTH * 6];

    unsigned int num_rows = PLANE_DEPTH - 1;
    unsigned int ind_per_row = PLANE_WIDTH * 2 + 2;
    unsigned int total_indices = (PLANE_DEPTH - 1) * (PLANE_WIDTH * 2 + 2);
    unsigned int num_segments = num_rows * (PLANE_WIDTH - 1 + 2) * 2; //zusammen mit den depricated +2
    unsigned int *indices = new unsigned int[(PLANE_DEPTH - 1) * (PLANE_WIDTH * 2 + 2)];
    face *faces_floor = new face[(PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2];
    float floor_roughness[] = { 24.f, 8.f, 2.f };
    float floor_amp_fit[] = { 3.f, 1.f, 0.3f };

    float* vertices_ceil = new float[PLANE_DEPTH * PLANE_WIDTH * 6];

    face* faces_ceil = new face[(PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2];
    float ceil_roughness[] = { 24.f,8.f, 3.f };
    float ceil_amp_fit [] = { 3.f, 1.f, 0.6 };

    //sticks data
    float* sticks_data = new float[NUM_STICKS*4];

    int** stick_col_mat = new int* [PLANE_DEPTH];
    for (int i = 0; i < PLANE_DEPTH; ++i) {
        stick_col_mat[i] = new int[PLANE_WIDTH];
        for(int j = 0; j < PLANE_WIDTH; j++){
            stick_col_mat[i][j] = 0;
        }
    }
    int radius_center = 15;
    for(int i = PLANE_WIDTH/2 - radius_center; i < PLANE_WIDTH/2 + radius_center + 1; i++){
        for(int j = PLANE_DEPTH/2 - radius_center; j < PLANE_DEPTH/2 + radius_center + 1; j++){
            if(glm::distance(glm::vec2(PLANE_WIDTH/2,PLANE_DEPTH/2), glm::vec2(i,j)) < radius_center) {
                stick_col_mat[i][j] = 1;
            }
        }
    }


    //camera variables
    float delta_time = 0.0f;
    float last_frame = 0.0f;
    float speed = 60;
    float momentum[6] = {0.0f};
    float period[6] = {0.0f};
    unsigned int pressed[6] = {0};

    //Plane position floor
    glm::vec3 pos_floor = glm::vec3(0.0, 0.0, 0.0);
    glm::mat4 model_floor = glm::mat4(1);
    model_floor= glm::translate(model_floor, pos_floor);

    //Plane position ceil
    glm::vec3 pos_ceil = glm::vec3(0.0, (float)PLANE_DIFF, 0.0);
    glm::mat4 model_ceil = glm::mat4(1);
    model_ceil = glm::translate(model_ceil, pos_ceil);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    HeightGenerator generator_floor(8.f, floor_roughness, floor_amp_fit, 3, 1, 0);
    HeightGenerator generator_ceil(9.f, ceil_roughness, ceil_amp_fit, 3, -1, 0);

    //generate vertices floor
    generateVertices(vertices_floor, generator_floor);
    generateVertices(vertices_floor_tmp, generator_floor);
    calculateNormals(vertices_floor, num_vertices, faces_floor, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
                     generator_floor,1);

    //generate vertices ceiling
    generateVertices(vertices_ceil, generator_ceil);
    generateVertices(vertices_ceil_tmp, generator_ceil);
    calculateNormals(vertices_ceil, num_vertices, faces_ceil, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2, generator_ceil,-1);

    generateIndices(indices, total_indices);

    //generate stick data
    generateSticks(sticks_data, stick_col_mat);
    


    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    camera cam(window, PLANE_WIDTH, PLANE_DEPTH, vertices_floor);


    unsigned int texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data_load = stbi_load("../data/sandstein.jpg",&width,&height,&nrChannels,0);
    if(data_load){

        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB32F,width, height,0,GL_RGB,GL_UNSIGNED_BYTE,data_load);
        glGenerateMipmap(GL_TEXTURE_2D);


    }else{
        std::cout<< "Bild konnte nicht geladen werden"<<std::endl;
    }


    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("base.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("base.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    

    glUseProgram(shaderProgram);
   
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "projection");
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_pos_point");
    int tex_loc = glGetUniformLocation(shaderProgram, "tex");
    int rand_light_loc = glGetUniformLocation(shaderProgram, "rand_light");
    int cameraPositon_loc = glGetUniformLocation(shaderProgram, "cameraPosition");
    int inner_radius_loc = glGetUniformLocation(shaderProgram, "inner_radius");
    int outer_radius_loc = glGetUniformLocation(shaderProgram, "outer_radius");
    int camera_dir_loc = glGetUniformLocation(shaderProgram, "cam_dir");
    int spotlight_activ_loc = glGetUniformLocation(shaderProgram, "spotlight_activ");

    glUniform1f(inner_radius_loc, glm::cos(glm::radians(25.f)));
    glUniform1f(outer_radius_loc, glm::cos(glm::radians(31.f)));

    glm::vec3 light_dir = glm::normalize(glm::vec3((float)PLANE_WIDTH/2.f, 20, (float)PLANE_DEPTH/2.f));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);


    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_FRAMEBUFFER_SRGB);

    unsigned int VAO[2];
    glGenVertexArrays(2,VAO);
    glBindVertexArray(VAO[0]);

    unsigned int VBO_floor = makeBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, 6 * num_vertices * sizeof(float), vertices_floor);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_floor);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //delete [] vertices;

    unsigned int IBO_floor = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, total_indices* sizeof(unsigned int), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_floor);
    //delete [] indices;

    glBindVertexArray(VAO[1]);
    unsigned int VBO_ceil = makeBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW,6 * num_vertices * sizeof(float), vertices_ceil);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ceil);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    unsigned int IBO_ceil = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_DYNAMIC_DRAW, total_indices*sizeof(unsigned int), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO_ceil);
    float growth_factor_start = 0.0f;
    float growth_time_sticks = 0.0f;

    //todo Entfernen
    std::random_device dev;
    std::mt19937 e2(dev());
    std::normal_distribution<> dist_light(0.9f, 0.1f);
    int lauf = 0;
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float current_Frame = glfwGetTime();
        delta_time = current_Frame -last_frame;
        if (growth_time_sticks < 450.f) {
            while (delta_time < 1.f) { //1.f
                Sleep(5);
                current_Frame = glfwGetTime();
                delta_time = current_Frame - last_frame;
            }
        }
        else {
            while (delta_time < 0.03333333f) {
                Sleep(5);
                current_Frame = glfwGetTime();
                delta_time = current_Frame - last_frame;
            }
        }
        
        last_frame=current_Frame;

        key_pressed(window,pressed);
        assign_momentum(pressed, momentum,period);
        processInput(window,delta_time*speed,momentum,period);

        glUseProgram(shaderProgram);

        glUniform1i(tex_loc, 0);

        glm::mat4 view;
        if (cam.is_not_dragging_w_momentum())
            cam.apply_after_momentum();
        view = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_floor[0][0]);
        glUniform3fv(cameraPositon_loc, 1, glm::value_ptr(cam.position()));
        glUniform3fv(camera_dir_loc, 1, glm::value_ptr(cam.getDirection()));
        glUniform1i(spotlight_activ_loc, spotlight);

        if(lauf % 8 == 0) {
            glUniform1f(rand_light_loc, dist_light(e2));
        }

        //Growth //soll 150.f sein

        if(growth_factor_start<=150.f) { //todo anpassen
            glBindBuffer(GL_ARRAY_BUFFER, VBO_floor);
            growth_plane(vertices_floor, vertices_floor_tmp, growth_factor_start, 150.f);
            calculateNormals(vertices_floor, num_vertices, faces_floor, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
                             generator_floor,1);
            
            glBufferSubData(GL_ARRAY_BUFFER,0,0,NULL);
            glBufferSubData(GL_ARRAY_BUFFER, 0 ,6 * num_vertices * sizeof(float),vertices_floor);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO_ceil);
            growth_plane(vertices_ceil, vertices_ceil_tmp, growth_factor_start, 150.f);
            calculateNormals(vertices_ceil, num_vertices, faces_ceil, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
                             generator_ceil,-1);
            
            glBufferSubData(GL_ARRAY_BUFFER,0,0,NULL);
            glBufferSubData(GL_ARRAY_BUFFER, 0  ,6 * num_vertices * sizeof(float),vertices_ceil);
            growth_factor_start++;
        }
        else if(growth_time_sticks < 450.f){ //450.f
            //Wachstum der Ebenen
            // ? if(growth_time_sticks < 30)
            growSticks(vertices_ceil, vertices_floor, sticks_data);
        
            generateDrops(vertices_ceil, vertices_floor);
        
            //ceiling
            glBindBuffer(GL_ARRAY_BUFFER, VBO_ceil);
            calculateNormals(vertices_ceil, num_vertices, faces_ceil, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
                generator_ceil,-1);
            glBufferSubData(GL_ARRAY_BUFFER,0,0,NULL);
            glBufferSubData(GL_ARRAY_BUFFER, 0  ,6 * num_vertices * sizeof(float),vertices_ceil);
        
            //floor
            glBindBuffer(GL_ARRAY_BUFFER, VBO_floor);
            calculateNormals(vertices_floor, num_vertices, faces_floor, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
               generator_floor,1);
        
            glBufferSubData(GL_ARRAY_BUFFER, 0, 0, NULL);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * num_vertices * sizeof(float), vertices_floor);
        
            growth_time_sticks++;
        }
        else {
            cam.set_vertices(vertices_floor);
        }
        glBindVertexArray(VAO[0]);

        glDrawElements(GL_TRIANGLE_STRIP, total_indices, GL_UNSIGNED_INT, (void*)0);
        // render plane

        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_ceil[0][0]);

        glBindVertexArray(VAO[1]);

        glDrawElements(GL_TRIANGLE_STRIP, total_indices, GL_UNSIGNED_INT,(void*)0);

        glfwSwapBuffers(window);
        // process window events

        glfwPollEvents();

        lauf++;
    }


    glfwTerminate();

}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}

void growth_plane(float* vertices, float* tmp_vertices, float growth_factor, float growth_range) {
    float factor = 1./pow(growth_range, 2) * pow(growth_factor,2);
    if (growth_factor <= growth_range) {
        for (int z = 0; z < PLANE_DEPTH; z++) {
            for (int x = 0; x < PLANE_DEPTH; x++) {
                vertices[(z * PLANE_WIDTH + x) * 6 + 1] =  factor * tmp_vertices[(z * PLANE_WIDTH + x) * 6 + 1];
            }
        }
    }
}

void generateVertices(float *vertices, HeightGenerator generator){
    for (unsigned j = 0; j < PLANE_DEPTH; j++) {

        for (unsigned i = 0; i < PLANE_WIDTH; i++) {
            vertices[(j * PLANE_WIDTH + i) * 6 + 0] = i;
            vertices[(j * PLANE_WIDTH + i) * 6 + 1] = generator.generateHeight(i, j);
            vertices[(j * PLANE_WIDTH + i) * 6 + 2] = j;

        }
    }
}

void calculateNormals(float* vertices, unsigned int v_size, face* faces, unsigned int f_size, HeightGenerator generator,int invert)
{
    /*
    for (unsigned j = 0; j < PLANE_DEPTH; j++) {

        for (unsigned i = 0; i < PLANE_WIDTH; i++) {
            vertices[(j * PLANE_WIDTH + i) * 6 + 0] = i;
            vertices[(j * PLANE_WIDTH + i) * 6 + 1] = generator.generateHeight(i, j);
            vertices[(j * PLANE_WIDTH + i) * 6 + 2] = j;

        }
    }
    */

    //flächen und deren Normale berechnen
    for (unsigned z = 0; z < PLANE_DEPTH - 1; z++) {
        for (unsigned x = 0; x < PLANE_WIDTH - 1; x++) {
            //coords rausholen und in faces speichern

            //pro Vertex
            //erstes Dreieck
            unsigned int face_index = (z * (PLANE_WIDTH - 1) + x) * 2 + 0;
            faces[face_index].vertices[0] = glm::vec3(x, vertices[(z * PLANE_DEPTH + x) * 6 + 1], z);
            faces[face_index].vertices[1] = glm::vec3(x, vertices[((z + 1) * PLANE_DEPTH + x) * 6 + 1], z + 1);
            faces[face_index].vertices[2] = glm::vec3(x + 1, vertices[((z + 1) * PLANE_DEPTH + x + 1) * 6 + 1], z + 1);
            faces[face_index].normal = glm::cross(faces[face_index].vertices[1] - faces[face_index].vertices[0], faces[face_index].vertices[2] - faces[face_index].vertices[0]);
            faces[face_index].space = 0.5f * glm::length(faces[face_index].normal);
            faces[face_index].normal = glm::normalize(faces[face_index].normal);

            //zweites Dreieck
            face_index = (z * (PLANE_WIDTH - 1) + x) * 2 + 1;
            faces[face_index].vertices[0] = glm::vec3(x, vertices[(z * PLANE_DEPTH + x) * 6 + 1], z);
            faces[face_index].vertices[1] = glm::vec3(x + 1, vertices[((z)*PLANE_DEPTH + x + 1) * 6 + 1], z);
            faces[face_index].vertices[2] = glm::vec3(x + 1, vertices[((z + 1) * PLANE_DEPTH + x + 1) * 6 + 1], z + 1);
            faces[face_index].normal = -glm::cross(faces[face_index].vertices[1] - faces[face_index].vertices[0], faces[face_index].vertices[2] - faces[face_index].vertices[0]);
            faces[face_index].space = 0.5f * glm::length(faces[face_index].normal);
            faces[face_index].normal = glm::normalize(faces[face_index].normal);

        }
    }

    //vertex normalen berechnen aus gewichtetem Durchschnitt, die vertices am Rand müssen gesondert betrachtet werden, erstmal egal, fürs große ganze
    //todo Randvertices normalen berechnen
    //jeder innere vertex grenzt an sechs verschiedene Dreiecke
    for (unsigned z = 1; z < PLANE_DEPTH - 1; z++) {
        for (unsigned x = 1; x < PLANE_WIDTH - 1; x++) {
            float total_space = 0.f;
            glm::vec3 normal = glm::vec3(0, 0, 0);
            unsigned face_index;
            //Dreiecke oben links von Punkt:
            //1.
            face_index = ((z - 1) * (PLANE_WIDTH - 1) + x - 1) * 2;
            total_space += faces[face_index].space;
            normal += faces[face_index].space * faces[face_index].normal;
            //2.
            face_index = ((z - 1) * (PLANE_WIDTH - 1) + x - 1) * 2 + 1;
            total_space += faces[face_index].space;
            normal += faces[face_index].space * faces[face_index].normal;
            //Dreieck oben rechts von Punkt:
            //1.
            face_index = ((z - 1) * (PLANE_WIDTH - 1) + x) * 2;
            total_space += faces[face_index].space;
            normal += faces[face_index].space * faces[face_index].normal;
            //Dreieck unten links von Punkt:
            //1.
            face_index = (z * (PLANE_WIDTH - 1) + x - 1) * 2 + 1;
            total_space += faces[face_index].space;
            normal += faces[face_index].space * faces[face_index].normal;
            //Dreiecke unten rechts von Punkt:
            //1.
            face_index = (z * (PLANE_WIDTH - 1) + x) * 2;
            total_space += faces[face_index].space;
            normal += faces[face_index].space * faces[face_index].normal;
            //2.
            face_index = (z * (PLANE_WIDTH - 1) + x) * 2 + 1;
            total_space += faces[face_index].space;
            normal += faces[face_index].space * faces[face_index].normal;

            normal /= total_space;
            normal = glm::normalize(normal);
            normal *= invert;
            vertices[(z * PLANE_WIDTH + x) * 6 + 3] = normal.x;
            vertices[(z * PLANE_WIDTH + x) * 6 + 4] = normal.y;
            vertices[(z * PLANE_WIDTH + x) * 6 + 5] = normal.z;
        }
    }
}

void generateIndices(unsigned int* indices, unsigned int ind_size) {

    //triangle strips
    unsigned i = 0;
    for (unsigned z = 0; z < PLANE_DEPTH - 1; z++) {
        for (unsigned x = 0; x < PLANE_WIDTH; x++) {
            indices[i++] = (z + 1) * PLANE_WIDTH + x;
            indices[i++] = z * PLANE_WIDTH + x;

        }
        //degenerate
        if (z != PLANE_DEPTH - 2) {
            indices[i] = indices[i - 1];
            i++;
            indices[i++] = (z + 2) * PLANE_WIDTH + 0;
        }
        else {
            indices[i] = indices[i - 1];
        }
    }
}

void growSticks(float* vertices, float* vertices_floor, float* sticks_data) {
    float radius, growth_fac;
    int  x_pos, z_pos;
    std::random_device dev;
    std::mt19937 e2(dev());
    //std::uniform_real_distribution<> dist_growth(0.001f, 0.2f);
    //todo time_growth entfernen oder nicht linear steigen lassen

    //iterate over sticks
    for (int i = 0; i < NUM_STICKS; i++) {
        //stick data
        x_pos = sticks_data[i * 4 + 0];
        z_pos = sticks_data[i * 4 + 1];
        radius = sticks_data[i * 4 + 2];
        int radius_ceiled = (int)ceil(radius);
        srand(time(NULL));
        growth_fac = sticks_data[i * 4 + 3] * 1.7f;
        //Wachstum nur, wenn Ebenen sich nicht berühren
        float distance_test = vertices[(z_pos * PLANE_DEPTH + x_pos) * 6 + 1] - vertices_floor[(z_pos * PLANE_DEPTH + x_pos) * 6 + 1] + PLANE_DIFF;

        if(distance_test > 0) {

            //Stalagtiten wachsen lassen
            for (int u = z_pos - radius_ceiled; u < z_pos + radius_ceiled + 1; u++) {
                for (int v = x_pos - radius_ceiled; v < x_pos + radius_ceiled + 1; v++) {
                    float distance_tmp = glm::distance(glm::vec2(v, u), glm::vec2(x_pos, z_pos));
                    if (distance_tmp <= radius) {

                        if (distance_tmp < 1.41f) {
                            vertices[(u * PLANE_DEPTH + v) * 6 + 1] -= growth_fac  * 1.1f;
                        } else if (distance_tmp < 2.0) {
                            vertices[(u * PLANE_DEPTH + v) * 6 + 1] -= growth_fac  * 1.f;
                        } else if (distance_tmp < 2.3) {
                            vertices[(u * PLANE_DEPTH + v) * 6 + 1] -= growth_fac  * 0.9f;
                        } else if (distance_tmp < 2.8) {
                            vertices[(u * PLANE_DEPTH + v) * 6 + 1] -= growth_fac  * 0.9f;
                        } else {
                            vertices[(u * PLANE_DEPTH + v) * 6 + 1] -= growth_fac  * 0.8f;
                        }
                    }
                }
            }


            //von unten stalagmiten wachsen lassen
            float height_diff = vertices[(z_pos * PLANE_DEPTH + x_pos) * 6 + 1] -
                                vertices_floor[(z_pos * PLANE_DEPTH + x_pos) * 6 + 1] + PLANE_DIFF;
            //radius abhängig von höhendifferenz, growth ebenfall
            float stalag_m_radius = height_diff * growth_fac / 2 + radius;
            float stalag_m_growth = growth_fac / stalag_m_radius * 2.5f;

            //test if point is in radius
            int approx_radius = (int) ceil(stalag_m_radius);
            float distance_tmp;
            for (int u = z_pos - approx_radius; u < z_pos + approx_radius + 1; u++) {
                for (int v = x_pos - approx_radius; v < x_pos + approx_radius + 1; v++) {
                    distance_tmp = glm::distance(glm::vec2(v, u), glm::vec2(x_pos, z_pos));
                    if (distance_tmp <= stalag_m_radius) {

                        if (distance_tmp <= 3.f / 5.f * stalag_m_radius) {
                            vertices_floor[(u * PLANE_DEPTH + v) * 6 + 1] += stalag_m_growth * 0.9;
                        } else if (distance_tmp <= 4.f / 5.f * stalag_m_radius) {
                            vertices_floor[(u * PLANE_DEPTH + v) * 6 + 1] += stalag_m_growth * 0.85f;
                        } else {
                            vertices_floor[(u * PLANE_DEPTH + v) * 6 + 1] += stalag_m_growth * 0.8f;
                        }
                    }
                }
            }
        }
    }
}

void generateSticks(float* sticks_data, int** stick_col_mat) {

    int num_sticks = NUM_STICKS;
    float x_base = 0.f;
    float z_base = 0.f;
    float radius_base = 0.f;

    float x_child = 0.f;
    float z_child = 0.f;
    float radius_child = 0.f;

    int sticks_cnt_tmp = 0;
    //float dist_center;
    int running_stick_index = 0;

    srand(time(NULL));
    while (num_sticks > 0) {
        //generate base circle

        //generate patch center x, z in interval [30, PLANEWIDTH -30]
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist_pw(40, PLANE_WIDTH - 40); // distribution in range [1, 6]

        std::mt19937 e2(dev());
        std::uniform_real_distribution<> dist_radius(12.f, 20.f);
        std::uniform_real_distribution<> dist1(0.f, 1.f);
        std::normal_distribution<> dist_growth(0.02f, 0.03f);

        dist_pw(rng);
        x_base = dist_pw(rng);
        z_base = dist_pw(rng);
        //x_base = rand() % (PLANE_WIDTH - 60) + 30;
        //z_base = rand() % (PLANE_WIDTH - 60) + 30;

        //generate radius from 8 to 18 as a float
        //radius_base = (float)(rand()) / (float)RAND_MAX * (float)(rand() % 11) + 8.f;
        radius_base = dist_radius(e2);

        //generate sticks_cnt_tmp from 3 to 6 for sticks in base circle
        sticks_cnt_tmp = rand() % 4 + 3;
        if (num_sticks < sticks_cnt_tmp) {
            sticks_cnt_tmp = num_sticks;
        }
        num_sticks -= sticks_cnt_tmp;

        //generate sticks_pos in circle and radius of stick
        for (int i = sticks_cnt_tmp; i > 0; i--) {
            //pos
            float dist_center = fmod((float)dist_pw(rng), radius_base);
            float angle = fmod((float)dist_pw(rng), 2 * M_PI);
            float x_pos_tmp = (int)(x_base + dist_center * cos(angle));
            float z_pos_tmp = (int)(z_base + dist_center * sin(angle));
            float radius_tmp = dist1(e2) * 2.f + 1.f;
            
            if (stick_col_set(stick_col_mat, x_pos_tmp, z_pos_tmp, radius_tmp)) {
                //xpos
                sticks_data[running_stick_index * 4 + 0] = x_pos_tmp;
                //zpos
                sticks_data[running_stick_index * 4 + 1] = z_pos_tmp;
                //radius from 1.1 to 3.1
                sticks_data[running_stick_index * 4 + 2] = radius_tmp;
                //growth speed
                sticks_data[running_stick_index * 4 + 3] = glm::clamp(dist_growth(e2), 0.001, 5.);


                running_stick_index++;
            }
            else {
                num_sticks++;
            }
        }

        //generate two child circles, with center within base circle
        for (int j = 0; j < 2; j++) {
            if (num_sticks <= 0)
                break;

            //generate sub center x, z on base_circle
            srand(time(NULL));
            float angle_center = fmod((float)dist_pw(rng), 2 * M_PI);
            x_child = x_base + radius_base * cos(angle_center);
            z_child = z_base + radius_base * sin(angle_center);

            //generate radius from 8 to 18 as a float
            radius_child = dist_radius(e2);

            //generate sticks_cnt_tmp from 3 to 6
            sticks_cnt_tmp = rand() % 4 + 3;
            if (num_sticks < sticks_cnt_tmp) {
                sticks_cnt_tmp = num_sticks;
            }
            num_sticks -= sticks_cnt_tmp;

            //generate sticks_pos in circle and radius of stick
            for (int i = sticks_cnt_tmp; i > 0; i--) {
                //pos
                float dist_center = fmod((float)dist_pw(rng), radius_child);
                float angle = fmod((float)dist_pw(rng), 2 * M_PI);
                float x_pos_tmp = (int)(x_child + dist_center * cos(angle));
                float z_pos_tmp = (int)(z_child + dist_center * sin(angle));
                float radius_tmp = dist1(e2) * 2.f + 1.f;

                if(stick_col_set(stick_col_mat, x_pos_tmp, z_pos_tmp, radius_tmp)) {
                    //xpos
                    sticks_data[running_stick_index * 4 + 0] = x_pos_tmp;
                    //zpos
                    sticks_data[running_stick_index * 4 + 1] = z_pos_tmp;
                    //radius from 1.1 to 3.1
                    sticks_data[running_stick_index * 4 + 2] = dist1(e2) * 2.f + 1.f;
                    //growth speed
                    sticks_data[running_stick_index * 4 + 3] = abs(dist_growth(e2));

                    running_stick_index++;
                }
                else{
                    num_sticks++;
                }
            }
        }



    }
}

bool stick_col_set(int** col_mat, int x, int z, float radius) {
    if(col_mat[x][z] == 1){
        return false;
    }
    if(radius < 1.41f){
        //"kreuz"
        for(int i = -1;i<2;i+=2){
            if(col_mat[x+i][z] == 1){
                return false;
            }
            if(col_mat[x][z + i] == 1){
                return false;
            }
        }
        //set 1
        //"kreuz"
        for(int i = -1;i<2;i+=2){
            col_mat[x+i][z] = 1;
            col_mat[x][z+i] = 1;
        }
    }
    else if(radius < 2.0f){
        //"kreuz"
        for(int i = -1;i<2;i+=2){
            if(col_mat[x+i][z] == 1){
                return false;
            }
            if(col_mat[x][z + i] == 1){
                return false;
            }
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                if(col_mat[x+i][z+j] == 1){
                    return false;
                }
            }
        }
        //set 1
        //"kreuz"
        for(int i = -1;i<2;i+=2){
            col_mat[x+i][z] = 1;
            col_mat[x][z+i] = 1;
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                col_mat[x+i][z+j] = 1;
            }
        }

    }
    else if(radius < 2.3f){
        //"kreuz" + zweites "kreuz" + "mitte"
        for(int i = -2;i<3;i++){
            if(col_mat[x+i][z] == 1){
                return false;
            }
            if(col_mat[x][z + i] == 1){
                return false;
            }
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                if(col_mat[x+i][z+j] == 1){
                    return false;
                }
            }
        }

        //set 1
        //"kreuz"
        for(int i = -2;i<3;i++){
            col_mat[x+i][z] = 1;
            col_mat[x][z+i] = 1;
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                col_mat[x+i][z+j] = 1;
            }
        }
    }
    else if(radius < 2.8f){
        //"kreuz" + zweites "kreuz" + "mitte"
        for(int i = -2;i<3;i++){
            if(col_mat[x+i][z] == 1){
                return false;
            }
            if(col_mat[x][z + i] == 1){
                return false;
            }
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                if(col_mat[x+i][z+j] == 1){
                    return false;
                }
            }
        }
        //"Halbdiagonale"
        for(int i = -2;i<3;i+=4){
            for(int j = -1;j<2;j+=2){
                if(col_mat[x+i][z+j] == 1){
                    return false;
                }
                if(col_mat[x+j][z+i] == 1){
                    return false;
                }
            }
        }

        //set 1
        //"kreuz"
        for(int i = -2;i<3;i++){
            col_mat[x+i][z] = 1;
            col_mat[x][z+i] = 1;
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                col_mat[x+i][z+j] = 1;
            }
        }
        //"Halbdiagonale"
        for(int i = -2;i<3;i+=4){
            for(int j = -1;j<2;j+=2){
                col_mat[x+i][z+j] = 1;
                col_mat[x+j][z+i] = 1;
            }
        }

    }
    else if(radius < 3.1f){
        //"kreuz" + zweites "kreuz" + "mitte" + drittes "Kreuz"
        for(int i = -3;i<4;i++){
            if(col_mat[x+i][z] == 1){
                return false;
            }
            if(col_mat[x][z + i] == 1){
                return false;
            }
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                if(col_mat[x+i][z+j] == 1){
                    return false;
                }
            }
        }
        //"Halbdiagonale"
        for(int i = -2;i<3;i+=4){
            for(int j = -1;j<2;j+=2){
                if(col_mat[x+i][z+j] == 1){
                    return false;
                }
                if(col_mat[x+j][z+i] == 1){
                    return false;
                }
            }
        }
        //zweite Diagonale
        for(int i = -2;i<3;i+=4){
            for(int j = -2;j<3;j+=4){
                if(col_mat[x+i][z+j] == 1){
                    return false;
                }
            }
        }

        //set 1
        //"kreuz"
        for(int i = -3;i<4;i++){
            col_mat[x+i][z] = 1;
            col_mat[x][z+i] = 1;
        }
        //diagonale
        for(int i = -1;i<2;i+=2){
            for(int j = -1;j<2;j+=2){
                col_mat[x+i][z+j] = 1;
            }
        }
        //"Halbdiagonale"
        for(int i = -2;i<3;i+=4){
            for(int j = -1;j<2;j+=2){
                col_mat[x+i][z+j] = 1;
                col_mat[x+j][z+i] = 1;
            }
        }
        //zweite diagonale
        for(int i = -2;i<3;i+=4){
            for(int j = -2;j<3;j+=4){
                col_mat[x+i][z+j] = 1;
            }
        }
    }
    return true;
}

void generateDrops(float* vertices_ceil, float* vertices_floor){

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> drop_pos_gen(20, PLANE_WIDTH - 20); // distribution in range [1, 6]
    std::uniform_int_distribution<> drop_cnt_gen(NUM_STICKS * 10 * 0.6f, NUM_STICKS * 10);

    std::mt19937 e2(dev());
    std::uniform_real_distribution<> drop_mass_gen(0.03f, 0.05f);

    int drop_x,drop_z;
    float drop_y;
    float drop_radius = 1.5f;
    float drop_mass;
    glm::vec2 drop_deri;
    float growth_fac;
    float sediment_fac = 0.1f;
    bool drop_down;
    int drop_cnt = drop_cnt_gen(rng);
    //int drop_cnt = 100;

    for(int i= 0;i<drop_cnt;i++){
        drop_x = drop_pos_gen(rng);
        drop_z = drop_pos_gen(rng);
        drop_y = vertices_ceil[(drop_z * PLANE_DEPTH + drop_x) * 6 + 1];

        drop_mass = abs(drop_mass_gen(e2));
        drop_deri = drop_derivation(vertices_ceil, drop_x, drop_z);
        growth_fac = glm::length(drop_deri);

        drop_down = true;

        while(drop_deri.x != 0 && drop_deri.y != 0){
            //Tropfen hat Rand erreicht und wird verworfen
            if(drop_x < 20 || drop_x > PLANE_WIDTH-20 || drop_z < 20 || drop_z > PLANE_DEPTH-20){
                drop_down = false;
                break;
            }
            if(drop_mass <= 0.0001){
                drop_down = false;
                break;
            }

            

//            if (growth_fac > 10) {
//                sediment_fac = 0.4;
//            }
//            else {
//                sediment_fac = 1.f / ((growth_fac)+1.f) * drop_mass;
//            }

            sediment_fac = 1.f / ((growth_fac)+1.f) * drop_mass;
            //sediment_fac = 0.1f * drop_mass;


            //drop einfluss bestimmen + wachstum
            for (int u = drop_z - ceil(drop_radius); u < drop_z + ceil(drop_radius) + 1; u++) {
                for (int v = drop_x - ceil(drop_radius); v <drop_x + ceil(drop_radius)+ 1; v++) {
                    //test ob in Kugelumgebung
                    float distance_tmp = glm::distance(glm::vec2(v,u), glm::vec2(drop_x, drop_z));
                    if (distance_tmp <= drop_radius) {

                        //zentrum
                        if (distance_tmp < 1.f) {
                            vertices_ceil[(u * PLANE_DEPTH + v) * 6 + 1] -= sediment_fac * 1.f;
                        }
                        //direkte nachbarn
                        else if (distance_tmp < 1.5f) {
                            vertices_ceil[(u * PLANE_DEPTH + v) * 6 + 1] -= sediment_fac * 0.95f;
                        }
                        //diagonale
                        else {
                            vertices_ceil[(u * PLANE_DEPTH + v) * 6 + 1] -= sediment_fac * 0.9f;
                        }
                    }
                }
            }


            drop_deri = glm::normalize(drop_deri);
            if(drop_deri.x > 0){
                drop_x = ceil(drop_x + drop_deri.x - 0.3f);
            }else{
                drop_x = floor(drop_x + drop_deri.x + 0.3f);
            }
            if(drop_deri.y > 0){
                drop_z = ceil(drop_z + drop_deri.y - 0.3f);
            }else{
                drop_z = floor(drop_z + drop_deri.y + 0.3f);
            }

            drop_mass -= sediment_fac;
            drop_deri = drop_derivation(vertices_ceil, drop_x, drop_z);
        }

        if(drop_down){
            //Wachstum des Bodens
            sediment_fac = 1.f/(growth_fac+1.f) * drop_mass;
            float height_diff = vertices_ceil[(drop_z * PLANE_DEPTH + drop_x) * 6 + 1] -
                                vertices_floor[(drop_z * PLANE_DEPTH + drop_x) * 6 + 1] + PLANE_DIFF;
            //radius abhängig von höhendifferenz, growth ebenfall
            float stalag_m_drop_radius = height_diff * sediment_fac / PLANE_DIFF * 2 + drop_radius;
            float stalag_m_drop_growth = sediment_fac / stalag_m_drop_radius * 2.5f;

            //test if point is in radius
            int approx_radius = (int) ceil(stalag_m_drop_radius);
            float distance_tmp;
            for (int u = drop_z - approx_radius; u < drop_z + approx_radius + 1; u++) {
                for (int v = drop_x - approx_radius; v < drop_x + approx_radius + 1; v++) {
                    distance_tmp = glm::distance(glm::vec2(v, u), glm::vec2(drop_x, drop_z));
                    if (distance_tmp <= stalag_m_drop_radius) {

                        if (distance_tmp <= 3.f / 5.f * stalag_m_drop_radius) {
                            vertices_floor[(u * PLANE_DEPTH + v) * 6 + 1] += stalag_m_drop_growth * 0.6;
                        } else if (distance_tmp <= 4.f / 5.f * stalag_m_drop_radius) {
                            vertices_floor[(u * PLANE_DEPTH + v) * 6 + 1] += stalag_m_drop_growth * 0.55f;
                        } else {
                            vertices_floor[(u * PLANE_DEPTH + v) * 6 + 1] += stalag_m_drop_growth * 0.5f;
                        }
                    }
                }
            }
        }
    }
}

glm::vec2 drop_derivation(float* vertices, float x, float z){
    int cent_x = (int)x;
    int cent_z = (int)z;


    float drop_y = vertices[(cent_z * PLANE_DEPTH + cent_x) * 6 +1];
    bool extrema = true;
    glm::mat3 sur_mat = glm::mat3(0.f);
    glm::mat3 sobel_hor = glm::mat3(0.f);
    glm::mat3 sobel_vert = glm::mat3(0.f);
    sobel_hor[0] = glm::vec3(1,2,1);
    sobel_hor[2] = glm::vec3(-1,-2,-1);
    sobel_vert[0] = glm:: vec3(-1,0,1);
    sobel_vert[1] = glm::vec3(-2,0,2);
    sobel_vert[2] = glm::vec3(-1,0,1);

    //test auf Punkt ist Tiefpunkt und Höhenwerte in Mat speichern
    for(int i = -1; i < 2; i++){
        for(int j = -1;j < 2; j++){
            sur_mat[i+1][j+1] = vertices[((cent_z + j) * PLANE_DEPTH + (cent_x + i)) * 6 +1];
            if(drop_y > sur_mat[i+1][j+1]){
                extrema = false;
            }
        }
    }
    if(extrema){
        return glm::vec2(0.0f,0.0f);
    }
    //sobel-filter
    float x_dir = compAdd(glm::matrixCompMult(sobel_hor, sur_mat));
    float z_dir = compAdd(glm::matrixCompMult(sobel_vert, sur_mat));

    return -glm::vec2(x_dir, z_dir);

}

float compAdd(glm::mat3 matrix){
    float sum = 0;
    for(int i = 0;i<3;i++) {
        for (int j = 0; j < 3; j++) {
            sum += matrix[i][j];
        }
    }
    return sum;
}

void processInput(GLFWwindow *window,float delta_time,float* momentum,float *period){
    int key[] ={GLFW_KEY_W,GLFW_KEY_S,GLFW_KEY_D,GLFW_KEY_A,GLFW_KEY_SPACE,GLFW_KEY_LEFT_SHIFT}; //alle zu testenden keys

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){ //shift beschleunigt
        delta_time *= 2.0f;
    }
    if(glfwGetKey(window,GLFW_KEY_TAB) == GLFW_PRESS){
        if(spotlight){
            spotlight = 0;
        }else{
            spotlight = 1;
        }
    }

    for (int i = 0; i < 6; i++) {
        if (period[i] > 0) {
            camera::keycallback(window, key[i], delta_time * momentum[i]);
        }
    }
}

void assign_momentum(unsigned int *pressed,float *momentum,float *period){
    for(int i = 0;i<6;i++) {
        if (pressed[i]) {
            if (period[i] < 5) {
                period[i] += 0.1f;
            }
            momentum[i] = 0.04f * pow(period[i], 2);
        } else {
            if (period[i] > 0) {
                period[i] -= 0.1f;
            }
            momentum[i] = 0.04f * pow(period[i], 2);
        }
    }
}

void key_pressed(GLFWwindow* window,unsigned int* pressed){
    int key[] ={GLFW_KEY_W,GLFW_KEY_S,GLFW_KEY_D,GLFW_KEY_A,GLFW_KEY_SPACE,GLFW_KEY_LEFT_SHIFT};
    for(int i=0;i<6;i++){
        if(glfwGetKey(window,key[i]) == GLFW_PRESS){
            pressed[i] = 1;
        }else{
            pressed[i] = 0;
        }
    }
}