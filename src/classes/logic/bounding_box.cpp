#include "bounding_box.h"

bool BoundingBox::intersects(const std::shared_ptr<BoundingBox>& other) const {
    return (min.x <= other->max.x && max.x >= other->min.x) &&
           (min.y <= other->max.y && max.y >= other->min.y) &&
           (min.z <= other->max.z && max.z >= other->min.z);
}

bool BoundingBox::contains(const glm::vec3& point) const {
    return (point.x >= min.x && point.x <= max.x) &&
           (point.y >= min.y && point.y <= max.y) &&
           (point.z >= min.z && point.z <= max.z);
}

bool BoundingBox::intersects_ray(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& t) const {
    glm::vec3 invDir = 1.0f / rayDir;
    glm::vec3 t1 = (min - rayOrigin) * invDir;
    glm::vec3 t2 = (max - rayOrigin) * invDir;
    
    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);
    
    float t_min = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float t_max = glm::min(glm::min(tmax.x, tmax.y), tmax.z);
    
    if (t_max < 0 || t_min > t_max) return false;
    
    t = t_min;
    return true;
}