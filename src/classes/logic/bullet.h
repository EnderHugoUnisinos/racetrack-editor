#pragma once

#include "../rendering/3D/obj3d.h"

class Bullet {
public:
    glm::vec3 position;
    glm::vec3 direction;
    float speed;
    float lifetime;
    float max_lifetime;
    bool active;
    bool reflected;
    
    std::shared_ptr<BoundingBox> bbox;

    Bullet(glm::vec3 pos, glm::vec3 dir, float spd = 20.0f, float life = 3.0f) 
        : position(pos), direction(glm::normalize(dir)), speed(spd), 
          lifetime(life), max_lifetime(life), active(true), reflected(false) 
    {
        bbox = std::make_shared<BoundingBox>();
        float bulletSize = 0.1f;
        bbox->min = glm::vec3(-bulletSize, -bulletSize, -bulletSize);
        bbox->max = glm::vec3(bulletSize, bulletSize, bulletSize);
        bbox->center = position;
        bbox->size = bbox->max - bbox->min;
        bbox->halfSize = bbox->size * 0.5f;
    }

    void update(float deltaTime);
    void reflect(const glm::vec3& normal);
    std::shared_ptr<BoundingBox> get_world_bbox() const;
};