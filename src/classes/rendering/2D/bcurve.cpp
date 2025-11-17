#include "bcurve.h"
#include <cmath>

void BSpline::addControlPoint(const glm::vec3& point) {
    controlPoints.push_back(point);
}

void BSpline::setControlPoints(const std::vector<glm::vec3>& points) {
    controlPoints = points;
}

void BSpline::setStepsPerSegment(int steps) {
    stepsPerSegment = steps;
}

std::vector<glm::vec3> BSpline::evaluateCurve() const {
    std::vector<glm::vec3> curvePoints;
    
    if (controlPoints.size() < 4) {
        return curvePoints;
    }
    
    int N = controlPoints.size();
    float inc = 1.0f / stepsPerSegment;
    
    for (int i = 0; i < N; i++) {
        for (float t = 0; t <= 1.0f; t += inc) {
            float X[4] = {
                controlPoints[i].x,
                controlPoints[(i + 1) % N].x,
                controlPoints[(i + 2) % N].x,
                controlPoints[(i + 3) % N].x
            };
            
            float Y[4] = {
                controlPoints[i].y,
                controlPoints[(i + 1) % N].y,
                controlPoints[(i + 2) % N].y,
                controlPoints[(i + 3) % N].y
            };
            
            float Z[4] = {
                controlPoints[i].z,
                controlPoints[(i + 1) % N].z,
                controlPoints[(i + 2) % N].z,
                controlPoints[(i + 3) % N].z
            };
            
            float t2 = t * t;
            float t3 = t2 * t;
            
            float B0 = (-t3 + 3*t2 - 3*t + 1) / 6.0f;
            float B1 = (3*t3 - 6*t2 + 4) / 6.0f;
            float B2 = (-3*t3 + 3*t2 + 3*t + 1) / 6.0f;
            float B3 = t3 / 6.0f;
            
            float x = B0 * X[0] + B1 * X[1] + B2 * X[2] + B3 * X[3];
            float y = B0 * Y[0] + B1 * Y[1] + B2 * Y[2] + B3 * Y[3];
            float z = B0 * Z[0] + B1 * Z[1] + B2 * Z[2] + B3 * Z[3];
            
            curvePoints.push_back(glm::vec3(x, y, z));
        }
    }
    
    return curvePoints;
}