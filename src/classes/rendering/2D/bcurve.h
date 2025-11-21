#pragma once
#include <vector>
#include <glm/glm.hpp>

class BSpline {
private:
    std::vector<glm::vec3> controlPoints;
    int stepsPerSegment = 20;

public:
    void addControlPoint(const glm::vec3& point);
    
    void set_control_points(const std::vector<glm::vec3>& points);
    
    std::vector<glm::vec3> evaluateCurve() const;
    
    void setStepsPerSegment(int steps);
};