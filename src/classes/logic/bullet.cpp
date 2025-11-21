#include "bullet.h"

void Bullet::update(float deltaTime) {
    if (!active) return;
    
    position += direction * speed * deltaTime;
    
    bbox->min = position - bbox->halfSize;
    bbox->max = position + bbox->halfSize;
    bbox->center = position;
    
    lifetime -= deltaTime;
    
    if (lifetime <= 0.0f) {
        active = false;
    }
}

void Bullet::reflect(const glm::vec3& normal) {
    direction = glm::reflect(direction, glm::normalize(normal));
    reflected = true;
    position += direction * 0.2f; 
}

std::shared_ptr<BoundingBox> Bullet::get_world_bbox() const {
    std::shared_ptr<BoundingBox> worldBBox = bbox;
    worldBBox->min = position - bbox->halfSize;
    worldBBox->max = position + bbox->halfSize;
    worldBBox->center = position;
    return worldBBox;
}