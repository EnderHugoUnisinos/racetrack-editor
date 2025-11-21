#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "obj3d.h"

class Scene{
public:
    std::vector<std::shared_ptr<Obj3D>> objects;

    void add_object(std::shared_ptr<Obj3D> object);
    void remove_object(int index);
    
    void update();

    void cleanup();
};
