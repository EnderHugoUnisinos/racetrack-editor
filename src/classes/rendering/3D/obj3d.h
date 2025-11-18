#pragma once

#include "mesh.h"
#include "animation.h"

class Obj3D {
public:
    bool buffers_created;
    std::string name;
    std::string obj_file;
    glm::mat4 transform;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Animation> animation;
    float animation_time = 0.0f;
    bool is_animated = false;

    Obj3D() : buffers_created(false), mesh(nullptr), transform(glm::mat4(1.0f)) {}
    
    void setup_buffers();
    void update(float deltaTime);
    void set_animation(std::shared_ptr<Animation> anim) { 
        animation = anim; 
        is_animated = (anim != nullptr);};
};
