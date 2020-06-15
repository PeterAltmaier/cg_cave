//
// Created by Antonia on 14.06.2020.
//
#include "common.hpp"
#include "shader.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int length);
void processInput(GLFWwindow* window);

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800,600,"Hello World!",NULL,NULL);
    //GLFWwindow* window = initOpenGL(800, 600, "Glorious Fail");
    if(window == NULL){
        std::cout << "Failed to create Window" <<std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "failed to initialize Glad"<< std::endl;
        return -1;
    }

    glViewport(0,0,800,600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    float vertices[]{
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };
    //Vertex Buffer
    unsigned int VBO;
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    //Shader und Programm erstellen und linken
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* shaderSource = loadShaderFile("OpenGLTraining_1.vert");
    glShaderSource(vertexShader,1,&shaderSource,NULL);
    glCompileShader(vertexShader);
    delete [] shaderSource;
    unsigned int fragmentShader = compileShader("OpenGLTraining_1.frag",GL_FRAGMENT_SHADER);
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);

    

    //Renderingloop
    while(!glfwWindowShouldClose(window)){
        processInput(window);

        glClearColor(0.f,0.6f,0.f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int length){
    glViewport(0,0,width, length);
}

void processInput(GLFWwindow* window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}