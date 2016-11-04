//
//  main.cpp
//  2DDrawingTemplate
//
//  Created by xiangyu on 03/11/2016.
//  Copyright © 2016 jxkj. All rights reserved.
//
// Std. Includes
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
//GLFW
#define GLEW_STATIC
#include <GL/glew.h>

//GLFW
#include <GLFW/glfw3.h>

//GL includes
#include "Shader.h"
#include "Camera.h"

//GLM mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Other Libs
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Triangulation_conformer_2.h>

#include <CGAL/Random.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Timer.h>
//Typedefs
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Iso_rectangle_2 Iso_rectangle_2;
typedef K::Segment_2 Segment_2;
typedef K::Ray_2 Ray_2;
typedef K::Line_2 Line_2;
typedef CGAL::Delaunay_triangulation_2<K>  Delaunay_triangulation_2;

typedef CGAL::Constrained_Delaunay_triangulation_2<K> CDT;
typedef CDT::Point Point;
typedef CDT::Vertex_handle Vertex_handle;
//Properties
GLuint screenWidth = 800, screenHeight = 600;

//Callback function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

//Other function prototypes
void generateData(GLfloat** vertices);
Delaunay_triangulation_2 delaunayTriangulation(std::vector<Point_2> pts);
CDT conformingGabrielDT(std::vector<Point> pts);
//Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

//Parameters
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLuint faceCount = 0;
GLuint vertsCount = 0;
//Namespaces

int main(int argc, const char * argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    //Create Periodic delaunay
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Periodic Delaunay", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    //Callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    //Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    //Init GLEW to setup the OpenGL Function pointers
    glewExperimental = GL_TRUE;
    glewInit();
    
    //Define the viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight);
    
    //Setup some OpenGL options
    //glEnable(GL_DEPTH_TEST);
    
    //Setup and compile shaders
    Shader ourShader("VertexShader.vert", "FragmentShader.frag");
    
    //Create vertices
//    GLfloat vertices[] = {
//        0.0f, 0.5f, 0.0f,
//        -0.5f, 0.0f, 0.0f,
//        0.5f, 0.0f, 0.0f
//    };
    GLfloat* vertices;
    generateData(&vertices);
//    for (int i = 0; i < 3 * vertsCount; i+=3)
//        std::cout << vertices[i] << " " << vertices[i+1] << " " << vertices[i + 2] << std::endl;
    //Setup VBO and VAO
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 3*vertsCount*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    
    //Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    //Load and create a texture
    //Game loop
    while(!glfwWindowShouldClose(window))
    {
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glfwPollEvents();
        Do_Movement();
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glClear(GL_COLOR_BUFFER_BIT);
        //Enable shader
        ourShader.Use();
        
        //Create camera transformation
        glm::mat4 view;
        view = camera.GetViewMatrix();
        
        glm::mat4 projection;
        projection = glm::perspective(camera.Zoom, (float)screenWidth/(float)screenHeight, 0.1f, 1000.0f);
        
        //Get the uniform locations
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projLoc = glGetUniformLocation(ourShader.Program, "projection");
        
        //Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        glBindVertexArray(VAO);
        glm::mat4 model;
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //GLfloat angle = 0.0f;
        //model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //Draw trangles
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, vertsCount);
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// Moves/alters the camera positions based on user input
void Do_Movement()
{
    // Camera controls
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    //cout << key << endl;
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key >= 0 && key < 1024)
    {
        if(action == GLFW_PRESS)
            keys[key] = true;
        else if(action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void generateData(GLfloat** vertices)
{
    typedef CGAL::Creator_uniform_2<double, Point_2> Creator;
    CGAL::Random random(7);
    CGAL::Random_points_in_square_2<Point_2, Creator> in_square(.5, random);
    int n = 100;
    std::vector<Point_2> pts;
    std::vector<Point> CGDT_pts;
    for (int i = 0 ; i < n ; i++)
    {
        Point_2 p = *in_square;
        in_square++;
        pts.push_back(Point_2(p.x() + .5, p.y() + .5));
        CGDT_pts.push_back(Point(p.x() + .5, p.y() + .5));
        //std::cout << pts[i].x() << " " << pts[i].y() << std::endl;
    }
    
    //Delaunay_triangulation_2 dt2 = delaunayTriangulation(pts);
    //Delaunay_triangulation_2::Finite_faces_iterator it;
    
    CDT dt2 = conformingGabrielDT(CGDT_pts);
    CDT::Finite_faces_iterator it;
    faceCount = dt2.number_of_faces();
    vertsCount = 3 * faceCount;
    *vertices = new GLfloat[9 * faceCount];
    std::cout << "Number of Faces: " << faceCount << std::endl;
    int index = 0;
    
    for (it = dt2.finite_faces_begin(); it != dt2.finite_faces_end(); it++)
    {
        
        (*vertices)[index    ] = dt2.triangle(it).vertex(0).x();
        (*vertices)[index + 1] = dt2.triangle(it).vertex(0).y();
        (*vertices)[index + 2] = 0.0f;
        
        index += 3;
        (*vertices)[index    ] = dt2.triangle(it).vertex(1).x();
        (*vertices)[index + 1] = dt2.triangle(it).vertex(1).y();
        (*vertices)[index + 2] = 0.0f;
        
        index += 3;
        (*vertices)[index    ] = dt2.triangle(it).vertex(2).x();
        (*vertices)[index + 1] = dt2.triangle(it).vertex(2).y();
        (*vertices)[index + 2] = 0.0f;
        
        index += 3;
    }
    
    std::ofstream outfile("output.out");
    if (!outfile.is_open()) return;
    for (int i = 0; i < 3 * vertsCount; i+=3)
    {
        outfile << (*vertices)[i] << " " <<(*vertices)[i + 1] << " " << (*vertices)[i + 2]<< std::endl;
    }

}

Delaunay_triangulation_2 delaunayTriangulation(std::vector<Point_2> pts)
{
   
    Delaunay_triangulation_2 dt2;
    dt2.insert(pts.begin(), pts.end());
    return dt2;
}

CDT conformingGabrielDT(std::vector<Point> pts)
{
    CDT cdt;
    for (int i = 0; i < pts.size(); i ++)
    {
        //insert points
        cdt.insert(pts[i]);
    }
    CGAL::make_conforming_Delaunay_2(cdt);
    
    CGAL::make_conforming_Gabriel_2(cdt);
    return cdt;
    
}
