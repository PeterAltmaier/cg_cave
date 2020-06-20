#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include <camera.hpp>
#include "mesh.hpp"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const float FOV = 90.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 1000.f;
const unsigned int PLANE_WIDTH = 1000;
const unsigned int PLANE_DEPTH = 1000;

glm::mat4 proj_matrix;


// called whenever the window gets resized
void
resizeCallback(GLFWwindow* window, int width, int height);

int
main(int, char* argv[]) {
    //vertex data
    
    unsigned int num_vertices = PLANE_DEPTH * PLANE_WIDTH;
    //float vertices[PLANE_DEPTH * PLANE_WIDTH *3] = { 0.f };
    float* vertices = new float[PLANE_DEPTH * PLANE_WIDTH * 6];
    unsigned int num_rows = PLANE_DEPTH - 1;
    unsigned int ind_per_row = PLANE_WIDTH * 2 + 2;
    unsigned int total_indices = (PLANE_DEPTH - 1) * (PLANE_WIDTH * 2 + 2);
    unsigned int num_segments = num_rows * (PLANE_WIDTH - 1 + 2) * 2;
    unsigned int *indices = new unsigned int[(PLANE_DEPTH - 1) * (PLANE_WIDTH * 2 + 2)];


    //Plane position
    glm::vec3 pos = glm::vec3(0.0, 1.0, 0.0);
    glm::mat4 model = glm::mat4(1);
    model= glm::translate(model, pos);
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);
    

    //generate vertices
    for (unsigned j = 0; j < PLANE_DEPTH; j++) {

        for (unsigned i = 0; i < PLANE_WIDTH; i++) {
            vertices[(j * PLANE_WIDTH + i) * 6 + 0] = i;
            vertices[(j * PLANE_WIDTH + i) * 6 + 1] = floor(rand()%100 / 94) * ((double)rand() / (RAND_MAX))*5;
            vertices[(j * PLANE_WIDTH + i) * 6 + 2] = j;
        }
    }

    //triangle strips
    unsigned i = 0;
    for (unsigned z = 0; z < PLANE_DEPTH-1; z++){
        for (unsigned x = 0; x < PLANE_WIDTH; x++) {
            indices[i++] = z * PLANE_WIDTH + x;
            indices[i++] = (z + 1) * PLANE_WIDTH + x;
        }
        //degenerate
        indices[i] = indices[i-1];
        i++;
        indices[i++] = (z + 1) * PLANE_WIDTH + 0;
    }


    GLFWwindow* window = initOpenGL(WINDOW_WIDTH, WINDOW_HEIGHT, argv[0]);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    camera cam(window);
    

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
    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);


    glEnable(GL_DEPTH_TEST);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, 6 * num_vertices * sizeof(float), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)3);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //delete [] vertices;

    unsigned int IBO = makeBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, total_indices* sizeof(unsigned int), indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    //delete [] indices;

    // rendering loop
    while (glfwWindowShouldClose(window) == false) {
        // set background color...
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        // and fill screen with it (therefore clearing the window)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        
        

        glm::mat4 view;
        view = cam.view_matrix();
        glUniformMatrix4fv(view_mat_loc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(proj_mat_loc, 1, GL_FALSE, &proj_matrix[0][0]);
        glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, &model[0][0]);
       
        glBindVertexArray(VAO);

        glDrawElements(GL_TRIANGLE_STRIP, total_indices, GL_UNSIGNED_INT, (void*)0);
        // render plane



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

