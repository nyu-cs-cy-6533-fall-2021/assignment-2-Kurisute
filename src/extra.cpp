// This example is heavily based on the tutorial at https://open.gl

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// OpenGL Mathematics Library
#include <glm/glm.hpp> // glm::vec3
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

// Timer
#include <chrono>
#include <iostream>
#include <string>
#include <map>
#include <math.h>

// Operation Modes:
enum mode {
    DEFAULT_MODE = 0,
    INSERT_MODE = 1,
    TRANSLATION_MODE = 2,
    DELETE_MODE = 3,
    COLOR_MODE = 4
};
mode Operation_mode = DEFAULT_MODE;

int SELECTED_OBJECT = -1;
int PRIM_SELECT = -1;
int SELECTED_VERTEX = -1;
int SELECTED_COLOR = 1;

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_C;

// Contains the vertex positions
//Eigen::MatrixXf V(2,3);
std::vector<glm::vec2> V(4);
std::vector<glm::vec3> C(4);
std::vector<glm::vec3> C_BACK_UP(3);
glm::mat4 VIEW;
glm::mat4 PROJECTION;
glm::mat4 TRANSFORMATION;

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}


glm::vec2 cursor_pos_in_window(GLFWwindow* window){
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    glm::vec4 p_screen(xpos,height-1-ypos,0,1);
    glm::vec4 p_canonical((p_screen.x/width)*2-1,(p_screen.y/height)*2-1,0,1);
    glm::vec4 p_world = glm::inverse(VIEW) * glm::inverse(PROJECTION) * p_canonical;
    // std::cout << p_world.x << p_world.y << std::endl;
    
    return glm::vec2(p_world.x,p_world.y);
}


double triangle_size(glm::vec2 v1,glm::vec2 v2,glm::vec2 v3){
    double size = 0.5 * glm::abs((v1.x - v3.x)*(v1.y - v2.y) - (v1.x - v2.x)*(v1.y - v3.y));
}


bool in_triangle(glm::vec2 pos,glm::vec2 v1,glm::vec2 v2,glm::vec2 v3){
    double total = triangle_size(v1,v2,v3);
    double a = triangle_size(pos,v2,v3);
    double b = triangle_size(pos,v1,v2);
    double c = triangle_size(pos,v1,v3);
    return abs(total - a - b - c) < 1e-6;
}


int nearest_vertex(glm::vec2 cursor_pos){
   int res = 3;
   double minDis = 1000;
   for(int i = 3;i < V.size()-1;i++){
       double dis = glm::distance(cursor_pos,V[i]);
       if(dis < minDis){
           minDis = dis;
           res = i;
       }
   }
   return res;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){

    glm::vec2 world_pos = cursor_pos_in_window(window);

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        switch (Operation_mode)
        {
        case INSERT_MODE:
            V.push_back(world_pos);
            C.push_back(glm::vec3(1.0f,1.0f,1.0f));
            std::cout << "V.size:" << V.size() << std::endl;
            break;
        case TRANSLATION_MODE:
            //get selected object
            for(int i = 3;i < V.size()-1;i+=3){
                if(in_triangle(world_pos,V[i],V[i+1],V[i+2])){
                    std::cout << "triangle:" << i << "selected." << std::endl;
                    SELECTED_OBJECT = i;
                    PRIM_SELECT = i;
                    C_BACK_UP[0] = C[SELECTED_OBJECT];
                    C_BACK_UP[1] = C[SELECTED_OBJECT+1];
                    C_BACK_UP[2] = C[SELECTED_OBJECT+2];
                    C[SELECTED_OBJECT] = glm::vec3(1, 1, 0);
                    C[SELECTED_OBJECT+1] = glm::vec3(1, 1, 0);
                    C[SELECTED_OBJECT+2] = glm::vec3(1, 1, 0);
                    VBO_C.update(C);
                    break;
                }
            }     
            break;
        case DELETE_MODE:
            {
                int delete_object = -1;
                for(int i = 3;i < V.size()-1;i+=3){
                    if(in_triangle(world_pos,V[i],V[i+1],V[i+2])){
                        std::cout << "triangle:" << i << "deleted." << std::endl;
                        delete_object = i;
                        break;
                    }
                }
                if(delete_object != -1) {
                    V.erase(V.begin()+delete_object,V.begin()+delete_object+3);
                    C.erase(C.begin()+delete_object,C.begin()+delete_object+3);
                }
            }
            break;
        case COLOR_MODE:
            {
                glm::vec2 cursor_pos = cursor_pos_in_window(window);
                SELECTED_VERTEX = nearest_vertex(cursor_pos);
                std::cout << "Vertex " << SELECTED_VERTEX << " selected." << std::endl;
            }
            break;
        default:
            break;
        }
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        switch (Operation_mode)
        {
            case TRANSLATION_MODE:
                C[SELECTED_OBJECT] = C_BACK_UP[0];
                C[SELECTED_OBJECT+1] = C_BACK_UP[1];
                C[SELECTED_OBJECT+2] = C_BACK_UP[2];
                VBO_C.update(C);
                SELECTED_OBJECT = -1;
                break;
            default:
                break;
        }
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if(action == GLFW_PRESS){
        switch (key)
        {
            case  GLFW_KEY_I:
                Operation_mode = INSERT_MODE;
                std::cout << "Insert mode activated." << std::endl;
                break;
            case GLFW_KEY_O:
                Operation_mode = TRANSLATION_MODE;
                std::cout << "Translate mode activated." << std::endl;
                break;
            case GLFW_KEY_P:
                Operation_mode = DELETE_MODE;
                std::cout << "Delete mode activated." << std::endl;
                break;
            case GLFW_KEY_C:
                Operation_mode = COLOR_MODE;
                std::cout << "Color mode activated." << std::endl;
                break;
            case GLFW_KEY_EQUAL:
                PROJECTION = glm::scale(PROJECTION,glm::vec3(1.2f,1.2f,1.0f));
                std::cout << "Camara zoom in by 20%." << std::endl;
                break;
            case GLFW_KEY_MINUS:
                PROJECTION = glm::scale(PROJECTION,glm::vec3(0.8f,0.8f,1.0f));
                std::cout << "Camara zoom out by 20% ." << std::endl;
                break;
            case GLFW_KEY_W:
                VIEW = glm::translate(VIEW,glm::vec3(0.0f,-0.2f,0.0f));
                break;
            case GLFW_KEY_A:
                VIEW = glm::translate(VIEW,glm::vec3(0.2f,0.0f,0.0f));
                break;
            case GLFW_KEY_S:
                VIEW = glm::translate(VIEW,glm::vec3(0.0f,0.2f,0.0f));
                break;
            case GLFW_KEY_D:
                VIEW = glm::translate(VIEW,glm::vec3(-0.2f,0.0f,0.0f));
                break;
            default:
                break;
        }
        //triangle translations of CPU Version
        if(Operation_mode == TRANSLATION_MODE){
            switch (key)
            {
                case GLFW_KEY_H:
                {
                    glm::mat4 trans = glm::mat4(1.0f);
                    glm::vec2 center = glm::vec2((V[PRIM_SELECT]+V[PRIM_SELECT+1]+V[PRIM_SELECT+2])/3.0f);
                    trans = glm::rotate(trans,glm::radians(10.0f),glm::vec3(0.0,0.0,1.0));
                    for(int i = PRIM_SELECT;i < PRIM_SELECT+3;i++){
                        V[i] = glm::vec2(trans * glm::vec4(V[i]-center,0.0f,0.0f)) + center;
                    }         
                }
                    break;
                case GLFW_KEY_J:
                {
                    glm::mat4 trans = glm::mat4(1.0f);
                    glm::vec2 center = glm::vec2((V[PRIM_SELECT]+V[PRIM_SELECT+1]+V[PRIM_SELECT+2])/3.0f);
                    trans = glm::rotate(trans,glm::radians(-10.0f),glm::vec3(0.0,0.0,1.0));
                    for(int i = PRIM_SELECT;i < PRIM_SELECT+3;i++){
                        V[i] = glm::vec2(trans * glm::vec4(V[i]-center,0.0f,0.0f)) + center;
                    }         
                }
                    break;
                case GLFW_KEY_K:
                {
                    glm::mat4 trans = glm::mat4(1.0f);
                    glm::vec2 center = glm::vec2((V[PRIM_SELECT]+V[PRIM_SELECT+1]+V[PRIM_SELECT+2])/3.0f);
                    trans = glm::scale(trans,glm::vec3(1.25,1.25,0.0));
                    for(int i = PRIM_SELECT;i < PRIM_SELECT+3;i++){
                        V[i] = glm::vec2(trans * glm::vec4(V[i]-center,0.0f,0.0f)) + center;
                    }         
                }
                    break;
                case GLFW_KEY_L:
                {
                    glm::mat4 trans = glm::mat4(1.0f);
                    glm::vec2 center = glm::vec2((V[PRIM_SELECT]+V[PRIM_SELECT+1]+V[PRIM_SELECT+2])/3.0f);
                    trans = glm::scale(trans,glm::vec3(0.75,0.75,0.0));
                    for(int i = PRIM_SELECT;i < PRIM_SELECT+3;i++){
                        V[i] = glm::vec2(trans * glm::vec4(V[i]-center,0.0f,0.0f)) + center;
                    }         
                }
                    break;
                default:
                    break;
            }
        }

        if(Operation_mode == COLOR_MODE && SELECTED_VERTEX != -1){
            switch (key)
            {
            case GLFW_KEY_1: //RED
                C[SELECTED_VERTEX] = glm::vec3(1.0,0.0,0.0);
                break;
            case GLFW_KEY_2: //GREEN
                C[SELECTED_VERTEX] = glm::vec3(0.0,1.0,0.0);
                break;
            case GLFW_KEY_3: //BLUE
                C[SELECTED_VERTEX] = glm::vec3(0.0,0.0,1.0);
                break;
            case GLFW_KEY_4: //YELLOW
                C[SELECTED_VERTEX] = glm::vec3(1.0,1.0,0.0);
                break;
            case GLFW_KEY_5: //PINK
                C[SELECTED_VERTEX] = glm::vec3(1.0,0.0,1.0);
                break;
            case GLFW_KEY_6: //CYAN
                C[SELECTED_VERTEX] = glm::vec3(0.0,1.0,1.0);
                break;
            case GLFW_KEY_7: //WHITE
                C[SELECTED_VERTEX] = glm::vec3(1.0,1.0,1.0);
                break;
            case GLFW_KEY_8:
                C[SELECTED_VERTEX] = glm::vec3(1.0,0.5,0.5);
                break;
            case GLFW_KEY_9:
                C[SELECTED_VERTEX] = glm::vec3(0.5,1.0,0.0);
                break;
            default:
                break;
            }
        }
    }
    // Upload the change to the GPU
    // VBO.update(V);
}


int main(void){
    GLFWwindow* window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window){
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    #ifndef __APPLE__
      glewExperimental = true;
      GLenum err = glewInit();
      if(GLEW_OK != err)
      {
        /* Problem: glewInit failed, something is seriously wrong. */
       fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      }
      glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
      fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
    #endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();
    V[0] = glm::vec2(0, 0);
    V[1] = glm::vec2(0, 0);
    V[2] = glm::vec2(0, 0);
    VBO.update(V);

    VBO_C.init();
    C[0] = glm::vec3(1.0, 0.0, 0.0);
    C[1] = glm::vec3(0.0, 1.0, 0.0);
    C[2] = glm::vec3(0.0, 0.0, 1.0);
    C[3] = glm::vec3(1.0, 1.0, 1.0);
    VBO_C.update(C);

    glm::vec2 last_cursor_pos;

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    float aspect_ratio = float(height)/float(width);
    VIEW = glm::scale(glm::mat4(1.f), glm::vec3(aspect_ratio, 1.f, 1.f));
    PROJECTION = glm::mat4(1.0f);
    TRANSFORMATION = glm::mat4(1.0f);

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar* vertex_shader =
            "#version 150 core\n"
                    "in vec2 position;"
                    "in vec3 color;"
                    "out vec3 f_color;"
                    "uniform mat4 view;"
                    "uniform mat4 projection;"
                    "uniform mat4 transformation;"
                    "void main()"
                    "{"
                    "    gl_Position = projection * view * transformation * vec4(position, 0.0, 1.0);"
                    "    f_color = color;"
                    "}";
    const GLchar* fragment_shader =
            "#version 150 core\n"
                    "in vec3 f_color;"
                    "out vec4 outColor;"
                    "uniform vec3 triangleColor;"
                    "void main()"
                    "{"
                    "    outColor = vec4(f_color, 1.0);"
                    "}";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader,fragment_shader,"outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position",VBO);
    program.bindVertexAttribArray("color",VBO_C);

    // Save the current time --- it will be used to dynamically change the triangle color
    auto t_start = std::chrono::high_resolution_clock::now();

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window)){
        // Bind your VAO (not necessary if you have only one)
        VAO.bind();

        // Bind your program
        program.bind();

        // Set the uniform value depending on the time difference
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, glm::value_ptr(VIEW));
        glUniformMatrix4fv(program.uniform("projection"), 1, GL_FALSE, glm::value_ptr(PROJECTION));
        glUniformMatrix4fv(program.uniform("tranformation"), 1, GL_FALSE, glm::value_ptr(TRANSFORMATION));

        // Clear the framebuffer
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::vec2 cursor_pos = cursor_pos_in_window(window);
        V[V.size()-1] = cursor_pos;

        //Making translations
        if(SELECTED_OBJECT != -1){
            glm::vec2 motion = cursor_pos - last_cursor_pos;
            TRANSFORMATION = glm::translate(TRANSFORMATION,glm::vec3(motion,0.0f));
        }

        VBO.update(V);
        
        VBO_C.update(C);

        //Draw triangles
        for(int i = 0;i < V.size()-3;i+=3){
            glDrawArrays(GL_TRIANGLES, i, 3);
        }

        // Draw the edge of editing-triangle (inserting process)      
        // glUniform3f(program.uniform("triangleColor"), 1.0f, 1.0f, 1.0f);
        if(V.size()%3 == 2)
            glDrawArrays(GL_LINES, V.size()-2, 2);       
        if(V.size()%3 == 0)
            glDrawArrays(GL_LINE_LOOP, V.size()-3, 3);

        last_cursor_pos = cursor_pos;
        
        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}

