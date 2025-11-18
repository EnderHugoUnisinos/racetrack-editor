#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include "../rendering/2D/bcurve.h"
#include "../rendering/3D/mesh.h"

class TrackEditor {
public:
    std::vector<glm::vec2> control_points;
    BSpline center_spline;
    float track_width;
    bool is_editing;

    TrackEditor(float width = 1.0f) : track_width(width), is_editing(true) {}

    void add_control_point(const glm::vec2& point);
    void clear_control_points();
    
    void generate_track_mesh(std::shared_ptr<Mesh>);
    
    void export_track_OBJ(const std::string& filename);
    void export_animation_file(const std::string& filename);
};