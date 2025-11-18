#include "animation.h"
#include <fstream>
#include <iostream>
#include <sstream>

void Animation::add_keyframe(const glm::vec3& position) {
    keyframes.push_back(position);
}

glm::vec3 Animation::get_position_at_time(float time) const {
    if (keyframes.empty()) return glm::vec3(0.0f);
    if (keyframes.size() == 1) return keyframes[0];
    
    if (looping) {
        time = fmod(time, duration);
    } else {
        time = glm::clamp(time, 0.0f, duration);
    }
    
    float segmentDuration = duration / (keyframes.size() - 1);
    int segment = static_cast<int>(time / segmentDuration);
    float t = (time - segment * segmentDuration) / segmentDuration;
    
    if (segment >= keyframes.size() - 1) {
        return keyframes.back();
    }
    
    return glm::mix(keyframes[segment], keyframes[segment + 1], t);
}

void Animation::load_from_file(const std::string& filename) {
    keyframes.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open animation file " << filename << std::endl;
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        glm::vec3 point;
        if (iss >> point.x >> point.y >> point.z) {
            keyframes.push_back(point);
        }
    }
    file.close();
}

void Animation::save_to_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    file << "# Animation keyframes\n";
    for (const auto& point : keyframes) {
        file << point.x << " " << point.y << " " << point.z << "\n";
    }
    file.close();
}