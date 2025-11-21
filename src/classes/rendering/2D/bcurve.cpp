#include "bcurve.h"
#include <cmath>

#include <iostream>
#include <vector>
#include <string>

void BSpline::addControlPoint(const glm::vec3& point) {
    controlPoints.push_back(point);
}

void BSpline::set_control_points(const std::vector<glm::vec3>& points) {
    controlPoints = points;
}

void BSpline::setStepsPerSegment(int steps) {
    stepsPerSegment = std::max(steps, 10);
}

std::vector<glm::vec3> BSpline::evaluateCurve() const {
    std::vector<glm::vec3> curvePoints;
    
    if (controlPoints.size() < 4) {
        return curvePoints;
    }
    
    int N = controlPoints.size();
    float inc = 1.0f / stepsPerSegment;
    
    for (int i = 0; i < N; i++) {
        for (float t = 0; t < 1.0f; t += inc) {
            int i0 = (i - 1 + N) % N;
            int i1 = i % N;
            int i2 = (i + 1) % N;
            int i3 = (i + 2) % N;
            
            float t2 = t * t;
            float t3 = t2 * t;
            
            float B0 = (-t3 + 3*t2 - 3*t + 1) / 6.0f;
            float B1 = (3*t3 - 6*t2 + 4) / 6.0f;
            float B2 = (-3*t3 + 3*t2 + 3*t + 1) / 6.0f;
            float B3 = t3 / 6.0f;
            
            glm::vec3 point = 
                controlPoints[i0] * B0 +
                controlPoints[i1] * B1 +
                controlPoints[i2] * B2 +
                controlPoints[i3] * B3;
            
            curvePoints.push_back(point);
        }
    }
    
    if (!curvePoints.empty() && controlPoints.size() >= 4) {
        curvePoints.push_back(curvePoints[0]);
    }
    
    return curvePoints;
}

