#pragma once

#include "mesh.h"

class Obj3D {
private:
    bool buffers_created;

public:
    std::string name;
    std::string file;
    glm::mat4 transform;
    std::shared_ptr<Mesh> mesh;
    
    // Add constructor
    Obj3D() : buffers_created(false), mesh(nullptr), transform(glm::mat4(1.0f)) {}
    
    void setup_buffers();
};
