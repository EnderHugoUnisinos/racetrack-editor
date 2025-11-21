#pragma once

#include "mesh.h"
#include "animation.h"
#include "../../logic/bounding_box.h"

class Obj3D {
public:
    bool active = true;

    bool buffers_created;
    std::string name;
    std::string obj_file;
    glm::mat4 transform;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Animation> animation;
    float animation_time = 0.0f;
    bool is_animated = false;

    bool selectable = false;
    bool eliminable;
    bool collidable;
    std::shared_ptr<BoundingBox> bbox;

    Obj3D() : buffers_created(false), mesh(nullptr), transform(glm::mat4(1.0f)) {
        bbox = std::make_shared<BoundingBox>();
    }
    
    void setup_buffers();
    void update(float deltaTime);
    void set_animation(std::shared_ptr<Animation> anim) { 
        animation = anim; 
        is_animated = (anim != nullptr);};
    void calculate_bbox();
    bool check_collision(const glm::vec3& point) const;
};
