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

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const float FOV = 90.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 1000.f;
const unsigned int PLANE_WIDTH = 300;
const unsigned int PLANE_DEPTH = 300;
const unsigned int NUM_STICKS = 30;

glm::mat4 proj_matrix;

struct face {
    glm::vec3 vertices[3];
    glm::vec3 normal;
    float space;
};

// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

void
calculateNormals(float* vertices, unsigned int v_size, face* faces, unsigned int f_size, HeightGenerator generator);

void
generateIndices(unsigned int *indices, unsigned int ind_size);

void processInput(GLFWwindow *window,float delta_time,float *momentum,float *period);

void assign_momentum(unsigned int *pressed,float *momentum,float *period);

void key_pressed(GLFWwindow *window,unsigned int *pressed);

void generateVertices(float *vertices, HeightGenerator generator);

void growth_plane(float* vertices, float growth_factor, float growth_range);

int
main(int, char* argv[]) {

    //vertex data
    
    unsigned int num_vertices = PLANE_DEPTH * PLANE_WIDTH;
    float* vertices_floor = new float[PLANE_DEPTH * PLANE_WIDTH * 6];
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
    float ceil_roughness[] = { 20.f,9.f, 3.f };
    float ceil_amp_fit[] = { 4.f, 1.f, 0.6f };

    //sticks data
    float* sticks_data = new float[NUM_STICKS*3];


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
    glm::vec3 pos_ceil = glm::vec3(0.0, 85.0, 0.0);
    glm::mat4 model_ceil = glm::mat4(1);
    model_ceil = glm::translate(model_ceil, pos_ceil);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    HeightGenerator generator_floor(8.f, floor_roughness, floor_amp_fit, 3, 1, 0);
    HeightGenerator generator_ceil(9.f, ceil_roughness, ceil_amp_fit, 3, -1, 0);

    //generate vertices floor
    generateVertices(vertices_floor, generator_floor);
    calculateNormals(vertices_floor, num_vertices, faces_floor, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
                     generator_floor);

    //generate vertices ceiling
    generateVertices(vertices_ceil, generator_ceil);
    calculateNormals(vertices_ceil, num_vertices, faces_ceil, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2, generator_ceil);

    generateIndices(indices, total_indices);

    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    camera cam(window);


    unsigned int texture;
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data_load = stbi_load("../data/kalk2.jpg",&width,&height,&nrChannels,0);
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
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
    int tex_loc = glGetUniformLocation(shaderProgram, "tex");
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);


    glEnable(GL_DEPTH_TEST);

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
    float growth_factor = 0.0f;
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float current_Frame = glfwGetTime();
        delta_time =current_Frame -last_frame;
        while (delta_time < 0.016666f) {
            Sleep(5);
            current_Frame = glfwGetTime();
            delta_time = current_Frame - last_frame;
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

        //Growth
        if(growth_factor<=1000) {
            growth_plane(vertices_floor, growth_factor, 1000.f);
            calculateNormals(vertices_floor, num_vertices, faces_floor, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
                             generator_floor);
            glBindVertexArray(VAO[0]);
            glBufferSubData(GL_ARRAY_BUFFER,0,6 * num_vertices * sizeof(float),vertices_floor);

            growth_plane(vertices_ceil, growth_factor, 1000.f);
            calculateNormals(vertices_ceil, num_vertices, faces_ceil, (PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2,
                             generator_ceil);
            glBindVertexArray(VAO[1]);
            glBufferSubData(GL_ARRAY_BUFFER,0,6 * num_vertices * sizeof(float),vertices_ceil);
            growth_factor++;
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
    }


    glfwTerminate();

}

void resizeCallback(GLFWwindow*, int width, int height)
{
    // set new width and height as viewport size
    glViewport(0, 0, width, height);
    proj_matrix = glm::perspective(FOV, static_cast<float>(width) / height, NEAR_VALUE, FAR_VALUE);
}

void growth_plane(float* vertices, float growth_factor, float growth_range) {
    if (growth_factor < growth_range) {
        for (int z = 0; z < PLANE_DEPTH; z++) {
            for (int x = 0; x < PLANE_DEPTH; x++) {
                vertices[(z * PLANE_WIDTH + x) * 6 + 1] *= growth_factor / growth_range;
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

void calculateNormals(float* vertices, unsigned int v_size, face* faces, unsigned int f_size, HeightGenerator generator)
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
            vertices[(z * PLANE_WIDTH + x) * 6 + 3] = normal.x;
            vertices[(z * PLANE_WIDTH + x) * 6 + 4] = normal.y;
            vertices[(z * PLANE_WIDTH + x) * 6 + 5] = normal.z;
        }
    }
}

void
generateIndices(unsigned int* indices, unsigned int ind_size) {

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

void processInput(GLFWwindow *window,float delta_time,float* momentum,float *period){
    int key[] ={GLFW_KEY_W,GLFW_KEY_S,GLFW_KEY_D,GLFW_KEY_A,GLFW_KEY_SPACE,GLFW_KEY_LEFT_SHIFT}; //alle zu testenden keys

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS){ //shift beschleunigt
        delta_time *= 2.0f;
    }

    for(int i=0;i<6;i++){
        if(period[i]>0){
            camera::keycallback(window,key[i],delta_time*momentum[i]);
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