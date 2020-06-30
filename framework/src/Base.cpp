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

glm::mat4 proj_matrix;


// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

void
generatePlane(float* vertices, unsigned int v_size, HeightGenerator generator);

void
generateIndices(unsigned int* indices, unsigned int ind_size);

int
main(int, char* argv[]) {
    //vertex data
    
    unsigned int num_vertices = PLANE_DEPTH * PLANE_WIDTH;
    float* vertices_floor = new float[PLANE_DEPTH * PLANE_WIDTH * 3];
    unsigned int num_rows = PLANE_DEPTH - 1;
    unsigned int ind_per_row = PLANE_WIDTH * 2 + 2;
    unsigned int total_indices = (PLANE_DEPTH - 1) * (PLANE_WIDTH * 2 + 2);
    unsigned int num_segments = num_rows * (PLANE_WIDTH - 1 + 2) * 2; //zusammen mit den depricated +2
    unsigned int *indices = new unsigned int[(PLANE_DEPTH - 1) * (PLANE_WIDTH * 2 + 2)];
    //face *faces_floor = new face[(PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2];
    float floor_roughness[] = { 24.f, 8.f, 2.f };
    float floor_amp_fit[] = { 3.f, 1.f, 0.3f };
   
    

    float* vertices_ceil = new float[PLANE_DEPTH * PLANE_WIDTH * 3];
    //face* faces_ceil = new face[(PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2];
    float ceil_roughness[] = { 20.f, 9.f, 3.f };
    float ceil_amp_fit[] = { 4.f, 1.f, 0.6f };
  
    

    //Plane position floor
    glm::vec3 pos_floor = glm::vec3(0.0, 0.0, 0.0);
    glm::mat4 model_floor = glm::mat4(1);
    model_floor= glm::translate(model_floor, pos_floor);

    //Plane position ceil
    glm::vec3 pos_ceil = glm::vec3(0.0, 85.0, 0.0);
    glm::mat4 model_ceil = glm::mat4(1);
    model_ceil = glm::translate(model_ceil, pos_ceil);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    HeightGenerator generator_floor(8.f, floor_roughness, floor_amp_fit, 3, 1, 20);
    HeightGenerator generator_ceil(9.f, ceil_roughness, ceil_amp_fit, 3, -1, 20);

    //generate vertices floor
    generatePlane(vertices_floor, num_vertices, generator_floor);
    
    //generate vertices ceiling
    generatePlane(vertices_ceil, num_vertices, generator_ceil);

    generateIndices(indices, total_indices);

    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    camera cam(window);


    // load and compile shaders and link program
    unsigned int vertexShader = compileShader("base_normal.vert", GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader("base.frag", GL_FRAGMENT_SHADER);
    unsigned int shaderProgram = linkProgram(vertexShader, fragmentShader);
    // after linking the program the shader objects are no longer needed
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    

    glUseProgram(shaderProgram);


    int growth_fac_loc = glGetUniformLocation(shaderProgram, "growth_fac");
    int model_mat_loc = glGetUniformLocation(shaderProgram, "model");
    int view_mat_loc = glGetUniformLocation(shaderProgram, "view");
    int proj_mat_loc = glGetUniformLocation(shaderProgram, "projection");
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
    int widht_loc = glGetUniformLocation(shaderProgram, "PLANE_WIDTH");
    int depth_loc = glGetUniformLocation(shaderProgram, "PLANE_DEPTH");
    int vertices_loc = glGetUniformLocation(shaderProgram, "vertices_tex");
    
    //load light_dir
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    //upload width and depth data
    glUniform1i(widht_loc, PLANE_WIDTH);
    glUniform1i(depth_loc, PLANE_DEPTH);

    //generate proj matrix
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);

    //create vertices_texture_floor
    //glEnable(GL_TEXTURE_1D);
    unsigned int vertices_floor_tex;
    glGenTextures(1, &vertices_floor_tex);
    //glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, vertices_floor_tex);
    std::cout << glGetError() << std::endl;
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, PLANE_DEPTH * PLANE_WIDTH, 0, GL_RGB, GL_FLOAT, vertices_floor);
    std::cout << glGetError() << std::endl; // returns 0 (no error)
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LEVEL, 0);
    glActiveTexture(GL_TEXTURE1);
    std::cout << glGetError() << std::endl; 
    //glBindTexture(GL_TEXTURE_1D, 1);
    glUniform1i(vertices_loc, 1);
    std::cout << glGetError() << std::endl; // returns 0 (no error)
    //glUniform1i(glGetUniformLocation(shaderProgram, "vertices_tex"), 0);

    //create vertices_texture_roof
    //unsigned int vertices_ceil_tex;
    //glGenTextures(1, &vertices_ceil_tex);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_1D, vertices_ceil_tex);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //
    //glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, PLANE_DEPTH, 0, GL_RGB, GL_FLOAT, vertices_ceil);
    //glUniform1i(glGetUniformLocation(shaderProgram, "vertices_tex"), 1);

    glEnable(GL_DEPTH_TEST);

    unsigned int VAO[2];
    glGenVertexArrays(2,VAO);
    glBindVertexArray(VAO[0]);

    unsigned int VBO_floor = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, 3 * num_vertices * sizeof(float), vertices_floor);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_floor);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
   
    glEnableVertexAttribArray(0);
    //delete [] vertices;

    unsigned int IBO_floor = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, total_indices* sizeof(unsigned int), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_floor);
    //delete [] indices;

    glBindVertexArray(VAO[1]);
    unsigned int VBO_ceil = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW,3 * num_vertices * sizeof(float), vertices_ceil);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_ceil);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);

    unsigned int IBO_ceil = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, total_indices*sizeof(unsigned int), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO_ceil);

    int growth = 1;
    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        
        
        glUniform1i(growth_fac_loc, growth);

        if (growth <= 30000) //todo in shader anpassen
            growth++;

        glm::mat4 view;
        view = cam.view_matrix();

        //floor render
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_floor[0][0]);
        glBindVertexArray(VAO[0]);

        //glUniform1i(glGetUniformLocation(shaderProgram, "vertices_tex"), 0);

        glDrawElements(GL_TRIANGLE_STRIP, total_indices, GL_UNSIGNED_INT, (void*)0);
       

        //ceil render
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model_ceil[0][0]);
       
        glBindVertexArray(VAO[1]);
        //glUniform1i(glGetUniformLocation(shaderProgram, "vertices_tex"), 1);
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

void generatePlane(float* vertices, unsigned int v_size, HeightGenerator generator)
{
    for (unsigned j = 0; j < PLANE_DEPTH; j++) {

        for (unsigned i = 0; i < PLANE_WIDTH; i++) {
            vertices[(j * PLANE_WIDTH + i) * 3 + 0] = (float)i / (float)PLANE_DEPTH ;
            vertices[(j * PLANE_WIDTH + i) * 3 + 1] = generator.generateHeight(i, j) / (float) PLANE_DEPTH;
            vertices[(j * PLANE_WIDTH + i) * 3 + 2] = (float)j /(float) PLANE_DEPTH;

        }
    }
    //todo test
    return;
    
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