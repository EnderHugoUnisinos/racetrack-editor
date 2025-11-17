#pragma once
#include <vector>
#include <glm/glm.hpp>

class BSpline {
private:
    std::vector<glm::vec3> controlPoints;
    int stepsPerSegment = 100;

public:
    void addControlPoint(const glm::vec3& point);
    
    void setControlPoints(const std::vector<glm::vec3>& points);
    
    std::vector<glm::vec3> evaluateCurve() const;
    
    void setStepsPerSegment(int steps);
};