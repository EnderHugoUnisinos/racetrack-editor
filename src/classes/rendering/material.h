#pragma once

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

class Material{
public:
    std::string tid;
    glm::vec3 ka, kd, ks;
    float Ns;
    std::string map_kd;
    unsigned int TID_mapKd;
    unsigned int TID_mapKs;
};