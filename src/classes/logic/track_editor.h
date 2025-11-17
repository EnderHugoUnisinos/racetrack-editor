#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include "../rendering/2D/bcurve.h"

class TrackEditor {
private:
    std::vector<glm::vec2> controlPoints;
    BSpline centerSpline;
    float trackWidth;
    bool isEditing;

public:
    TrackEditor(float width = 5.0f) : trackWidth(width), isEditing(true) {}

    void addControlPoint(const glm::vec2& point);
    void clearControlPoints();
    std::vector<glm::vec2> getControlPoints() const { return controlPoints; }
    
    void generateTrackMesh(std::vector<glm::vec3>& vertices, 
                          std::vector<glm::vec2>& texCoords,
                          std::vector<glm::vec3>& normals,
                          std::vector<unsigned int>& indices);
    
    void exportTrackOBJ(const std::string& filename);
    void exportAnimationFile(const std::string& filename);
    
    void setEditing(bool editing) { isEditing = editing; }
    bool getEditing() const { return isEditing; }
    void setTrackWidth(float width) { trackWidth = width; }
};