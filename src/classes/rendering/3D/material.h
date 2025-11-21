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
    std::string name;
    glm::vec3 ambient = glm::vec3(0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f);
    glm::vec3 specular = glm::vec3(1.0f);
    float shininess = 32.0f;

    bool needs_texture_loading() const {
        return (!diffuseMap.empty() && diffuse_texture == 0) ||
               (!specularMap.empty() && specular_texture == 0) ||
               (!normalMap.empty() && normal_texture == 0);
    }
    
    unsigned int diffuse_texture = 0;
    unsigned int specular_texture = 0;
    unsigned int normal_texture = 0;
    
    std::string diffuseMap;
    std::string specularMap;
    std::string normalMap;
    
    bool has_diffuse_texture() const { return diffuse_texture != 0; }
    bool has_specular_texture() const { return specular_texture != 0; }
    bool has_normal_texture() const { return normal_texture != 0; }
    
    void cleanup();
};