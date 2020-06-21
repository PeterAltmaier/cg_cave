#include <chrono>
#include <glm/gtx/transform.hpp>
#include "common.hpp"
#include "shader.hpp"
#include "buffer.hpp"
#include <camera.hpp>
#include "mesh.hpp"
#include <algorithm>
#include <iterator>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const float FOV = 90.f;
const float NEAR_VALUE = 0.1f;
const float FAR_VALUE = 1000.f;
const unsigned int PLANE_WIDTH = 100;
const unsigned int PLANE_DEPTH = 100;

glm::mat4 proj_matrix;

struct face {
    glm::vec3 vertices[3];
    glm::vec3 normal;
    float space;
};

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
    unsigned int num_segments = num_rows * (PLANE_WIDTH - 1 + 2) * 2; //zusammen mit den depricated +2
    unsigned int *indices = new unsigned int[(PLANE_DEPTH - 1) * (PLANE_WIDTH * 2 + 2)];
    face *faces = new face[(PLANE_DEPTH - 1) * (PLANE_WIDTH - 1) * 2];

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

   //flächen und deren Normale berechnen
   for (unsigned z = 0; z < PLANE_DEPTH - 1; z++) {
       for(unsigned x =0 ; x < PLANE_WIDTH-1; x++) {
           //coords rausholen und in faces speichern
   
           //pro Vertex
           //erstes Dreieck
           unsigned int face_index = (z * (PLANE_WIDTH-1) + x) * 2+0;
           faces[face_index].vertices[0] = glm::vec3(x, vertices[(z * PLANE_DEPTH + x) * 6 + 1], z);
           faces[face_index].vertices[1] = glm::vec3(x, vertices[((z + 1) * PLANE_DEPTH + x) * 6 + 1], z + 1);
           faces[face_index].vertices[2] = glm::vec3(x + 1, vertices[((z + 1) * PLANE_DEPTH + x+1) * 6 + 1], z + 1);
           faces[face_index].normal =glm::cross(faces[face_index].vertices[1] - faces[face_index].vertices[0], faces[face_index].vertices[2] - faces[face_index].vertices[0]);
           faces[face_index].space = 0.5f * glm::length(faces[face_index].normal);
           faces[face_index].normal = glm::normalize(faces[face_index].normal);
   
           //zweites Dreieck
           face_index = (z * (PLANE_WIDTH - 1) + x) * 2 + 1;
           faces[face_index].vertices[0] = glm::vec3(x, vertices[(z * PLANE_DEPTH + x) * 6 + 1], z);
           faces[face_index].vertices[1] = glm::vec3(x+1, vertices[((z ) * PLANE_DEPTH + x+1) * 6 + 1], z );
           faces[face_index].vertices[2] = glm::vec3(x + 1, vertices[((z + 1) * PLANE_DEPTH + x+1) * 6 + 1], z + 1);
           faces[face_index].normal = -glm::cross(faces[face_index].vertices[1] - faces[face_index].vertices[0], faces[face_index].vertices[2] - faces[face_index].vertices[0]);
           faces[face_index].space = 0.5f * glm::length(faces[face_index].normal);
           faces[face_index].normal = glm::normalize(faces[face_index].normal);
          
           }
   }
   
   //vertex normalen berechnen aus gewichtetem Durchschnitt, die vertices am Rand müssen gesondert betrachtet werden, erstmal egal, fürs große ganze
   //todo Randvertices normalen berechnen
   //jeder innere vertex grenzt an sechs verschiedene Dreiecke
   for (unsigned z = 1; z < PLANE_DEPTH-1; z++) {
       for (unsigned x = 1; x < PLANE_WIDTH-1; x++) {
           float total_space = 0.f;
           glm::vec3 normal = glm::vec3(0,0,0);
           unsigned face_index;
           //Dreiecke oben links von Punkt:
           //1.
           face_index = ((z-1) * (PLANE_WIDTH - 1) + x-1) * 2;
           total_space += faces[face_index].space;
           normal += faces[face_index].space * faces[face_index].normal;
           //2.
           face_index = ((z - 1) * (PLANE_WIDTH - 1) + x - 1) * 2 +1;
           total_space += faces[face_index].space;
           normal += faces[face_index].space * faces[face_index].normal;
           //Dreieck oben rechts von Punkt:
           //1.
           face_index = ((z - 1) * (PLANE_WIDTH - 1) + x ) * 2;
           total_space += faces[face_index].space;
           normal += faces[face_index].space * faces[face_index].normal;
           //Dreieck unten links von Punkt:
           //1.
           face_index = (z  * (PLANE_WIDTH - 1) + x - 1) * 2 +1;
           total_space += faces[face_index].space;
           normal += faces[face_index].space * faces[face_index].normal;
           //Dreiecke unten rechts von Punkt:
           //1.
           face_index = (z  * (PLANE_WIDTH - 1) + x) * 2;
           total_space += faces[face_index].space;
           normal += faces[face_index].space * faces[face_index].normal;
           //2.
           face_index = (z * (PLANE_WIDTH - 1) + x ) * 2 + 1;
           total_space += faces[face_index].space;
           normal += faces[face_index].space * faces[face_index].normal;
           
           normal /= total_space;
           normal = glm::normalize(normal);
           vertices[(z * PLANE_WIDTH + x) * 6 + 3] = normal.x;
           vertices[(z * PLANE_WIDTH + x) * 6 + 4] = normal.y;
           vertices[(z * PLANE_WIDTH + x) * 6 + 5] = normal.z;
       }
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
    int light_dir_loc = glGetUniformLocation(shaderProgram, "light_dir");
    glm::vec3 light_dir = glm::normalize(glm::vec3(1.0, 1.0, 1.0));
    glUniform3fv(light_dir_loc, 1, &light_dir[0]);

    proj_matrix = glm::perspective(FOV, 1.f, NEAR_VALUE, FAR_VALUE);


    glEnable(GL_DEPTH_TEST);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = makeBuffer(GL_ARRAY_BUFFER, GL_STATIC_DRAW, 6 * num_vertices * sizeof(float), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
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

