#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>

class Animation {
public:
    std::string name;
    std::vector<glm::vec3> keyframes;
    std::vector<glm::vec3> points;
    float duration = 10.0f;
    bool looping = true;
    
    void add_keyframe(const glm::vec3& position);
    glm::vec3 get_position_at_time(float time) const;
    void load_from_file(const std::string& filename);
    void save_to_file(const std::string& filename) const;
};