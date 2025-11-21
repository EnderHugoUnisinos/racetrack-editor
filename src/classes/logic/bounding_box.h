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
#include <algorithm>
#include <map>

class BoundingBox {
public:
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 center;
    glm::vec3 size;
    glm::vec3 halfSize;

    BoundingBox() : min(0.0f), max(0.0f), center(0.0f), size(0.0f) {}

    bool intersects(const std::shared_ptr<BoundingBox>& other) const;
    bool contains(const glm::vec3& point) const;
    bool intersects_ray(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float& t) const;
};