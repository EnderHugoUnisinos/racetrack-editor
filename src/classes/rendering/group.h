#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <iostream>

#include "face.h"
#include "material.h"

class Group {
public:
    Group(const std::string& group_name = ""): name(group_name), VAO(0), VBO(0) {};

    std::string name;
    std::vector<Face*> faces;
    Material *material;
    int vert_count;
    
    GLuint VAO;
    GLuint VBO;
};