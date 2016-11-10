//
//  main.cpp
//  2DDrawingTemplate
//
//  Created by xiangyu on 03/11/2016.
//  Copyright © 2016 jxkj. All rights reserved.
//
// Std. Includes
#define CGAL_MESH_2_OPTIMIZER_VERBOSE
//#define CGAL_MESH_2_OPTIMIZERS_DEBUG
//#define CGAL_MESH_2_SIZING_FIELD_USE_BARYCENTRIC_COORDINATES

#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
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

//CGAL Libs
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_plus_2.h>
#include <CGAL/Triangulation_conformer_2.h>
#include <CGAL/Delaunay_mesher_2.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Delaunay_mesh_size_criteria_2.h>

#include <CGAL/lloyd_optimize_mesh_2.h>

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


typedef CGAL::Triangulation_vertex_base_2<K> Vb;
typedef CGAL::Delaunay_mesh_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> Tds;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, Tds> CDT;
typedef CGAL::Constrained_triangulation_plus_2<CDT> CDTP;
typedef CGAL::Delaunay_mesh_size_criteria_2<CDT> Criteria;
typedef CGAL::Delaunay_mesher_2<CDT, Criteria> Mesher;

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
CDTP conformingGabrielDT(std::vector<Point> pts);
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

bool toggle = true;
bool flag = false;
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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Triangulation2", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    
    //Callback functions
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    //Options
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(window, screenWidth/2, screenHeight/2);
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
    GLuint VBO[2], VAO[2];
    glGenVertexArrays(1, &VAO[0]);
    glGenBuffers(1, &VBO[0]);
    
    glBindVertexArray(VAO[0]);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, 3*vertsCount*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    
    //Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    //Load and create a texture
    //Game loop
    while(!glfwWindowShouldClose(window))
    {
        if (flag)
        {
            generateData(&vertices);
            glGenVertexArrays(1, &VAO[1]);
            glGenBuffers(1, &VBO[1]);
            
            glBindVertexArray(VAO[1]);
            
            glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
            glBufferData(GL_ARRAY_BUFFER, 3*vertsCount*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
            
            //Position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);
            
            glBindVertexArray(0);
            flag = false;
        }
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
        
        if (flag)
            glBindVertexArray(VAO[1]);
        else
            glBindVertexArray(VAO[0]);
        glm::mat4 model;
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        //GLfloat angle = 0.0f;
        //model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        //Draw CGCDT
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, vertsCount);
        glBindVertexArray(0);
        
//        glBindVertexArray(VAO[1]);
//        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//        //Draw CCDT
//        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//        glDrawArrays(GL_TRIANGLES, 0, vertsCount);
//        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1, &VAO[0]);
    glDeleteBuffers(1, &VBO[0]);
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
    if (key == GLFW_KEY_N)
    {
        flag = true;
    }
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
//    typedef CGAL::Creator_uniform_2<double, Point_2> Creator;
//    CGAL::Random random(7);
//    CGAL::Random_points_in_square_2<Point_2, Creator> in_square(.5, random);
//    int n = 100;
//    std::vector<Point_2> pts;
    std::vector<Point> CGDT_pts;
//    for (int i = 0 ; i < n ; i++)
//    {
//        Point_2 p = *in_square;
//        in_square++;
//        pts.push_back(Point_2(p.x() + .5, p.y() + .5));
//        CGDT_pts.push_back(Point(p.x() + .5, p.y() + .5));
//        //std::cout << pts[i].x() << " " << pts[i].y() << std::endl;
//    }
    
    //Delaunay_triangulation_2 dt2 = delaunayTriangulation(pts);
    //Delaunay_triangulation_2::Finite_faces_iterator it;
    
    CDTP dt2 = conformingGabrielDT(CGDT_pts);
    CDTP::Finite_faces_iterator it;
    faceCount = dt2.number_of_faces();
    vertsCount = 3 * faceCount;
    *vertices = new GLfloat[9 * faceCount];
    std::cout << "Number of Faces: " << faceCount << std::endl;
    int index = 0;
    
    for (it = dt2.finite_faces_begin(); it != dt2.finite_faces_end(); it++)
    {
        if (it->is_in_domain())
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

CDTP conformingGabrielDT(std::vector<Point> pts)
{
    CDTP cdt;
    std::list<Point> list_of_seeds;
    std::ifstream infile("Points.txt");
    std::string line;
    std::string delimiter = ",";
    std::vector<Point> data;
    std::vector<int> identifier;
    if (infile.is_open())
    {
        //size_t pos = 0;
        std::string token;
        while (std::getline(infile, line))
        {
            float x = std::stof(line.substr(0, line.find(delimiter)));
            line.erase(0, line.find(delimiter) + delimiter.length());
            float y = std::stof(line.substr(0, line.find(delimiter)));
            line.erase(0, line.find(delimiter) + delimiter.length());
            int id = std::stoi(line.substr(0, line.find(delimiter)));
            data.push_back(Point(x, y));
            identifier.push_back(id);
            //std::cout << x << " " << y << " " << id << std::endl;
        }
        infile.close();
    }
    
//    Assumptions: 1. all points in one region are in spatial order.
//                 2. No bad data points. Meaning that all data entry should follow the (float, float, int) format.
//                 3. No intersections between regions.
//                 4. All regions must have at least two points.
    Vertex_handle prev = cdt.insert(data[0]);
    Vertex_handle start = prev;
    Vertex_handle current;
    int i;
    for (i = 1; i < data.size(); i++)
    {
        current = cdt.insert(data[i]);
        if (identifier[i] == identifier[i-1])
            cdt.insert_constraint(prev, current);
        else
            std::cout << glm::sqrt(glm::pow(data[i-2].x() - data[i-1].x(), 2) + glm::pow(data[i-2].y() - data[i-1].y(), 2)) << std::endl;
        prev = current;
    }
    std::cout << glm::sqrt(glm::pow(data[i-2].x() - data[i-1].x(), 2) + glm::pow(data[i-2].y() - data[i-1].y(), 2)) << std::endl;
    //cdt.insert_constraint(prev, start);
//    float l = 4;
//    for (int i = 0; i < 16;  i++)
//    {
//        Vertex_handle va = cdt.insert(Point(-l,0));
//        Vertex_handle vb = cdt.insert(Point(0,-l));
//        Vertex_handle vc = cdt.insert(Point(l,0));
//        Vertex_handle vd = cdt.insert(Point(0,l));
//        
//        cdt.insert_constraint(va, vb);
//        cdt.insert_constraint(vb, vc);
//        cdt.insert_constraint(vc, vd);
//        cdt.insert_constraint(vd, va);
//        
//        Vertex_handle ve = cdt.insert(Point(-l/2.0, -l/2.0));
//        Vertex_handle vf = cdt.insert(Point(l/2.0, -l/2.0));
//        Vertex_handle vg = cdt.insert(Point(l/2.0, l/2.0));
//        Vertex_handle vh = cdt.insert(Point(-l/2.0, l/2.0));
//        
//        cdt.insert_constraint(ve, vf);
//        cdt.insert_constraint(vf, vg);
//        cdt.insert_constraint(vg, vh);
//        cdt.insert_constraint(vh, ve);
//        
//        list_of_seeds.push_back(Point(l*(3.0/4.0), 0));
//        list_of_seeds.push_back(Point(l*(-3.0/4.0), 0));
//        list_of_seeds.push_back(Point(0, l*(3.0/4.0)));
//        list_of_seeds.push_back(Point(0, l*(-3.0/4.0)));
//        
//        l/=2;
//    }
//    Vertex_handle vf = cdt.insert(Point(10, 0));
//    Vertex_handle va = cdt.insert(Point(-4,0));
//    Vertex_handle vb = cdt.insert(Point(0,-4));
//    Vertex_handle vc = cdt.insert(Point(4,0));
//    Vertex_handle vd = cdt.insert(Point(0,4));
//    //Vertex_handle ve = cdt.insert(Point(4, 0));
//    //Vertex_handle vf = cdt.insert(Point(10, 10));
//    
////    Vertex_handle ve = cdt.insert(Point(-2, -2));
////    Vertex_handle vf = cdt.insert(Point(2, -2));
////    Vertex_handle vg = cdt.insert(Point(2, 2));
////    Vertex_handle vh = cdt.insert(Point(-2, 2));
//    
//    //cdt.insert_constraint(ve, vd);
//    
//    cdt.insert_constraint(va, vb);
//    cdt.insert_constraint(vb, vc);
//    cdt.insert_constraint(vc, vd);
//    cdt.insert_constraint(vd, va);
//    
//    //Vertex_handle vf = cdt.insert(Point(2, 0.6));
//    cdt.insert_constraint(vf, vd);
//    cdt.insert_constraint(vf, vc);
//    cdt.insert_constraint(vf, vb);
    //cdt.insert_constraint(vf, ve);
//            Vertex_handle prev = cdt.insert(Point(1, 0));
//            Vertex_handle start = prev;
//            Vertex_handle current;
//            for (int i = 1; i < 100; i++)
//            {
//                float degree = i*(glm::two_pi<float>()/100);
//                //std::cout << "degree: " << degree << " " << glm::cos(90) <<  std::endl;
//                current = cdt.insert(Point(glm::cos(degree), glm::sin(degree)));
//                cdt.insert_constraint(prev, current);
//                //std::cout << prev->point().x() << " " << prev->point().y() << " " << current->point().x() << " " << current->point().y() << std::endl;
//                prev = current;
//            }

//
//    cdt.insert_constraint(ve, vf);
//    cdt.insert_constraint(vf, vg);
//    cdt.insert_constraint(vg, vh);
//    cdt.insert_constraint(vh, ve);
    

    
//    list_of_seeds.push_back(Point(3, 0));
//    list_of_seeds.push_back(Point(-3, 0));
//    list_of_seeds.push_back(Point(0, 3));
//    list_of_seeds.push_back(Point(0, -3));
//
//    for (int i = 0; i < pts.size(); i ++)
//    {
//        //insert points
//        cdt.insert(pts[i]);
//    }
//    
//    cdt.insert_constraint(pts[1], pts[2]);
//    cdt.insert_constraint(pts[2], pts[3]);
//    cdt.insert_constraint(pts[3], pts[4]);
//    cdt.insert_constraint(pts[4], pts[1]);
    std::cout << "Number of vertices: " << cdt.number_of_vertices() << std::endl;
    
    std::cout << "Meshing the triangulation..." << std::endl;
    CGAL::refine_Delaunay_mesh_2(cdt, Criteria(0.125 , 0.05));
    //Mesher mesher(cdt);
    //cdt.delete_vertex(va);
    //mesher.refine_mesh();
    std::cout << "Number of vertices: " << cdt.number_of_vertices() << std::endl;
    
    std::cout << "Meshing with new criterias..." << std::endl;
    // 0.125 is the default shape bound. It corresponds to abound 20.6 degree.
    // 0.5 is the upper bound on the length of the longuest edge.
    // See reference manual for Delaunay_mesh_size_traits_2<K>.
    //mesher.set_criteria(Criteria(0.125, 0.5));
    //mesher.refine_mesh();
    //Vertex_handle prev = cdt.insert(Point(1, 0));
    //if (flag)
    //{
//        std::vector<Vertex_handle> vh;
//        for (int i = 0; i < 100; i++)
//        {
//            float degree = i*(360/100);
//            vh.push_back(cdt.insert(Point(glm::cos(degree), glm::sin(degree))));
//        
//        //cdt.insert_constraint(prev, current);
//        //prev = current;
//        }
    //}
    //std::vector<Vertex_handle>::iterator vhit = vh.begin();
//    Vertex_handle prev = vh[0];
//    Vertex_handle current;
//    //vhit++;
//    for (int i = 1;i < vh.size(); i++)
//    {
//        current = vh[i];
//        cdt.insert_constraint(prev, current);
//        prev = current;
//    }
//    CDTP::Finite_faces_iterator fit;
//    std::vector<Vertex_handle> vv;
//    std::vector<Vertex_handle>::iterator vvit;
//    for (fit = cdt.finite_faces_begin(); fit != cdt.finite_faces_end(); fit++)
//    {
//        Point x = cdt.triangle(fit).vertex(0);
//        Point y = cdt.triangle(fit).vertex(1);
//        Point z = cdt.triangle(fit).vertex(2);
//        bool flag = false;
//        if (x.x() * x.x() + x.y() * x.y() < 1)
//        {
//            vvit = find(vv.begin(), vv.end(), fit->vertex(0));
//            if (vvit == vv.end())
//            {
//                //cdt.delete_vertex(fit->vertex(0));
//                vv.push_back(fit->vertex(0));
//            }
//            flag = true;
//        }
//        if (y.x() * y.x() + y.y() * y.y() < 1)
//        {
//            vvit = find(vv.begin(), vv.end(), fit->vertex(1));
//            if (vvit == vv.end())
//            {
//                //cdt.delete_vertex(fit->vertex(1));
//                vv.push_back(fit->vertex(1));
//            }
//            flag = true;
//        }
//        if (z.x() * z.x() + z.y() * z.y() < 1)
//        {
//            vvit = find(vv.begin(), vv.end(), fit->vertex(2));
//            if (vvit == vv.end())
//            {
//                //cdt.delete_vertex(fit->vertex(2));
//                vv.push_back(fit->vertex(2));
//            }
//            flag = true;
//        }
//        if (flag)
//            fit->set_in_domain(false);
////        if (flag)
////            cdt.delete_face(fit);
//        //if (vit->point())
//    }
//    vv.clear();
//    std::vector<Vertex_handle> vh;
//    CDTP buf;
//    
//    CDTP::Finite_vertices_iterator vit;
//    for (vit = cdt.finite_vertices_begin(); vit != cdt.finite_vertices_end(); vit++)
//    {
//        buf.insert(vit->point());
//    }
    //cdt.insert(Point(5, 0));
    //cdt.insert(Point(0,0));
    
//    Vertex_handle prev = vh[0];
//    Vertex_handle current;
//    //vhit++;
//    for (int i = 1;i < vh.size(); i++)
//    {
//        current = vh[i];
//        cdt.insert_constraint(prev, current);
//        prev = current;
//    }
    //cdt.insert(Point(3.5 , 0));
    //CGAL::refine_Delaunay_mesh_2(cdt, Criteria(0.125, 0.5));
    //mesher.refine_mesh();
    //mesher.set_seeds(list_of_seeds.begin(), list_of_seeds.end(), true, true);
    //mesher.refine_mesh();

    
//    if (!flag)
//    {
//        Vertex_handle prev = cdt.insert(Point(1, 0));
//        Vertex_handle start = prev;
//        Vertex_handle current;
//        for (int i = 1; i < 100; i++)
//        {
//            float degree = i*(glm::two_pi<float>()/100);
//            //std::cout << "degree: " << degree << " " << glm::cos(90) <<  std::endl;
//            current = cdt.insert(Point(glm::cos(degree), glm::sin(degree)));
//            //cdt.insert_constraint(prev, current);
//            //std::cout << prev->point().x() << " " << prev->point().y() << " " << current->point().x() << " " << current->point().y() << std::endl;
//            //prev = current;
//        }
//        //cdt.insert(current, start);
//    }
    
//    for (int i = 0; i < 100; i++)
//    {
//        cdt.insert(Point(i*(4/100), 0));
//    }
    std::cout << "Number of vertices: " << cdt.number_of_vertices() << std::endl;
    
    
    
    //mesher.refine_mesh();
//    std::cout << "Run Lloyd optimization...";
//    CGAL::lloyd_optimize_mesh_2(cdt,
//                                CGAL::parameters::max_iteration_number = 10);
//    std::cout << " done." << std::endl;
    return cdt;
}
