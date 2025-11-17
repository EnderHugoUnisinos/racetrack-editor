#include "track_editor.h"
#include <fstream>
#include <iostream>
#include <algorithm>

void TrackEditor::addControlPoint(const glm::vec2& point) {
    controlPoints.push_back(point);
    
    // Converter para 3D para a BSpline
    std::vector<glm::vec3> points3D;
    for (const auto& p : controlPoints) {
        points3D.push_back(glm::vec3(p.x, p.y, 0.0f));
    }
    centerSpline.setControlPoints(points3D);
}

void TrackEditor::clearControlPoints() {
    controlPoints.clear();
    std::vector<glm::vec3> empty;
    centerSpline.setControlPoints(empty);
}

void TrackEditor::generateTrackMesh(std::vector<glm::vec3>& vertices, 
                                   std::vector<glm::vec2>& texCoords,
                                   std::vector<glm::vec3>& normals,
                                   std::vector<unsigned int>& indices) {
    if (controlPoints.size() < 4) return;

    auto centerPoints = centerSpline.evaluateCurve();
    if (centerPoints.size() < 2) return;

    vertices.clear();
    texCoords.clear();
    normals.clear();
    indices.clear();

    // Gerar pontos internos e externos
    std::vector<glm::vec3> innerPoints;
    std::vector<glm::vec3> outerPoints;

    for (size_t i = 0; i < centerPoints.size(); i++) {
        glm::vec3 current = centerPoints[i];
        glm::vec3 next = centerPoints[(i + 1) % centerPoints.size()];
        
        glm::vec3 tangent = glm::normalize(next - current);
        glm::vec3 normal = glm::vec3(-tangent.y, tangent.x, 0.0f);
        
        innerPoints.push_back(current - normal * trackWidth);
        outerPoints.push_back(current + normal * trackWidth);
    }

    // Gerar malha
    for (size_t i = 0; i < innerPoints.size(); i++) {
        // Adicionar vértices
        vertices.push_back(innerPoints[i]);
        vertices.push_back(outerPoints[i]);
        
        // Coordenadas de textura
        float u = static_cast<float>(i) / innerPoints.size();
        texCoords.push_back(glm::vec2(u, 0.0f)); // Interno
        texCoords.push_back(glm::vec2(u, 1.0f)); // Externo
        
        // Normais (apontando para cima)
        normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
        normals.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // Gerar índices
    int segments = innerPoints.size();
    for (int i = 0; i < segments; i++) {
        int next_i = (i + 1) % segments;
        
        // Primeiro triângulo
        indices.push_back(i * 2);         // inner i
        indices.push_back(next_i * 2);    // inner next
        indices.push_back(i * 2 + 1);     // outer i
        
        // Segundo triângulo  
        indices.push_back(i * 2 + 1);     // outer i
        indices.push_back(next_i * 2);    // inner next
        indices.push_back(next_i * 2 + 1);// outer next
    }
}

void TrackEditor::exportTrackOBJ(const std::string& filename) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    
    generateTrackMesh(vertices, texCoords, normals, indices);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    file << "# Racetrack exported from editor\n";
    
    // Escrever vértices (invertendo Y com Z para o visualizador 3D)
    for (const auto& v : vertices) {
        file << "v " << v.x << " " << v.z << " " << -v.y << "\n";
    }
    
    // Escrever coordenadas de textura
    for (const auto& tc : texCoords) {
        file << "vt " << tc.x << " " << tc.y << "\n";
    }
    
    // Escrever normais
    for (const auto& n : normals) {
        file << "vn " << n.x << " " << n.z << " " << -n.y << "\n";
    }
    
    // Escrever faces
    for (size_t i = 0; i < indices.size(); i += 3) {
        file << "f " << indices[i] + 1 << "/" << indices[i] + 1 << "/" << indices[i] + 1 << " "
                   << indices[i + 1] + 1 << "/" << indices[i + 1] + 1 << "/" << indices[i + 1] + 1 << " "
                   << indices[i + 2] + 1 << "/" << indices[i + 2] + 1 << "/" << indices[i + 2] + 1 << "\n";
    }
    
    file.close();
}

void TrackEditor::exportAnimationFile(const std::string& filename) {
    auto centerPoints = centerSpline.evaluateCurve();
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    file << "# Animation points for racetrack\n";
    for (const auto& point : centerPoints) {
        // Inverter Y com Z para o visualizador 3D
        file << point.x << " " << point.z << " " << -point.y << "\n";
    }
    
    file.close();
}