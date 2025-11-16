#ifndef OBJ3D_H
#define OBJ3D_H

#include "mesh.h"

class Obj3D {
private:
    bool buffers_created;

public:
    std::string name;
    std::string file;
    glm::mat4 transform;
    Mesh* mesh;
    void setup_buffers();
};
#endif