#pragma once
#include <vector>
#include <glm/glm.hpp>

class BSpline {
public:
    BSpline(int degree = 3);
    
    void addControlPoint(const glm::vec3& point);
    void setControlPoints(const std::vector<glm::vec3>& points);
    void clearControlPoints();
    
    void setDegree(int degree);
    int getDegree() const;
    
    std::vector<glm::vec3> generate_curve_points(int segments = 100) const;
    glm::vec3 evaluate(float t) const;
    
    const std::vector<glm::vec3>& get_control_points() const;
    const std::vector<float>& getKnots() const;

private:
    void generateKnotVector();
    int findKnotInterval(float t) const;
    float basisFunction(int i, int k, float t) const;

    std::vector<glm::vec3> controlPoints;
    std::vector<float> knots;
    int degree;
    bool dirty;
};