#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <algorithm>

#include "obj3dwriter.h"
#include "track_editor.h"

void TrackEditor::add_control_point(const glm::vec2& point) {
    control_points.push_back(point);
    
    // Converter para 3D para a BSpline
    std::vector<glm::vec3> points;
    for (const auto& p : control_points) {
        points.push_back(glm::vec3(p.x, 0.0f, p.y));
    }
    center_spline.set_control_points(points);
}

void TrackEditor::pop_back_control_points(){
    if (control_points.size()) {
        control_points.pop_back();
        std::vector<glm::vec3> points;
        for (const auto& p : control_points) {
            points.push_back(glm::vec3(p.x, 0.0f, p.y));
        }
        center_spline.set_control_points(points);
    }
}

void TrackEditor::clear_control_points() {
    control_points.clear();
    std::vector<glm::vec3> empty;
    center_spline.set_control_points(empty);
}

void TrackEditor::generate_track_mesh(std::shared_ptr<Mesh> mesh) {
    float textureScale = 0.2f; 
    if (control_points.size() < 4) {
        std::cout << "Need at least 4 control points for track generation" << std::endl;
        return;
    }

    auto centerPoints = center_spline.evaluateCurve();
    if (centerPoints.size() < 2) {
        std::cout << "Not enough curve points generated" << std::endl;
        return;
    }

    if (!material) {
        material = std::make_shared<Material>();
        material->name = "track_material";
        std::string texture_path = "road.jpg";
        material->diffuseMap = texture_path;
    }

    if (!mesh->groups.size()) {
        auto group = std::make_shared<Group>("track_group");
        group->material = material;
        mesh->groups.push_back(group);
    }

    // Clear existing mesh data
    mesh->verts.clear();
    mesh->mappings.clear();
    mesh->normals.clear();
    mesh->processed_verts.clear();
    mesh->processed_mappings.clear();
    mesh->processed_normals.clear();

    std::vector<glm::vec3> innerPoints;
    std::vector<glm::vec3> outerPoints;
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> normals;

    // Pre-calculate smooth tangents and normals
    for (size_t i = 0; i < centerPoints.size(); i++) {
        glm::vec3 prev = centerPoints[(i - 1 + centerPoints.size()) % centerPoints.size()];
        glm::vec3 current = centerPoints[i];
        glm::vec3 next = centerPoints[(i + 1) % centerPoints.size()];
        
        // Calculate smooth tangent using Catmull-Rom style
        glm::vec3 tangent = glm::normalize(next - prev);
        tangents.push_back(tangent);
        
        // Calculate normal in XZ plane (perpendicular to tangent)
        glm::vec3 normal = glm::normalize(glm::cross(tangent, glm::vec3(0.0f, 1.0f, 0.0f)));
        normals.push_back(normal);
    }

    // Generate inner and outer points
    for (size_t i = 0; i < centerPoints.size(); i++) {
        innerPoints.push_back(centerPoints[i] - normals[i] * track_width * 0.5f);
        outerPoints.push_back(centerPoints[i] + normals[i] * track_width * 0.5f);
    }

    // Generate vertices, texture coordinates, and normals
    for (size_t i = 0; i < centerPoints.size(); i++) {
        // Calculate surface normal (pointing upward for flat track)
        glm::vec3 surfaceNormal(0.0f, 1.0f, 0.0f);
        
        // Add inner vertex
        mesh->verts.push_back(innerPoints[i]);
        mesh->mappings.push_back(glm::vec2(static_cast<float>(i) * textureScale, 0.0f));
        mesh->normals.push_back(surfaceNormal);
        
        // Add outer vertex
        mesh->verts.push_back(outerPoints[i]);
        mesh->mappings.push_back(glm::vec2(static_cast<float>(i) * textureScale, 1.0f));
        mesh->normals.push_back(surfaceNormal);
    }
    //manually remove last vertices and replace with starting ones ???
    mesh->verts.pop_back();
    mesh->mappings.pop_back();
    mesh->normals.pop_back();
    mesh->verts.pop_back();
    mesh->mappings.pop_back();
    mesh->normals.pop_back();

    mesh->verts.push_back(mesh->verts[0]);
    mesh->mappings.push_back(mesh->mappings[0]);
    mesh->normals.push_back(mesh->normals[0]);
    mesh->verts.push_back(mesh->verts[1]);
    mesh->mappings.push_back(mesh->mappings[1]);
    mesh->normals.push_back(mesh->normals[1]);

    mesh->groups[0]->VAO = 0;
    mesh->groups[0]->VBO = 0;
    mesh->groups[0]->vert_count = 0;
    mesh->groups[0]->faces.clear();

    // Generate proper triangle indices (QUAD STRIPS)
    int numSegments = centerPoints.size();
    for (int i = 0; i < numSegments; i++) {
        int next_i = (i + 1) % numSegments;
        
        int inner_current = i * 2;
        int outer_current = i * 2 + 1;
        int inner_next = next_i * 2;
        int outer_next = next_i * 2 + 1;
        
        //inner_current -> inner_next -> outer_current
        auto face1 = std::make_shared<Face>();
        face1->verts = {inner_current, inner_next, outer_current};
        face1->textures = {inner_current, inner_next, outer_current};
        face1->normals = {inner_current, inner_next, outer_current};
        mesh->groups[0]->faces.push_back(face1);
        
        //outer_current -> inner_next -> outer_next
        auto face2 = std::make_shared<Face>();
        face2->verts = {outer_current, inner_next, outer_next};
        face2->textures = {outer_current, inner_next, outer_next};
        face2->normals = {outer_current, inner_next, outer_next};
        mesh->groups[0]->faces.push_back(face2);
    }

    mesh->process_data();

    std::cout << "Generated track mesh: " << mesh->verts.size() << " vertices, " 
              << mesh->groups[0]->faces.size() * 3 << " indices (" 
              << mesh->groups[0]->faces.size() << " triangles)" << std::endl;
}

void TrackEditor::export_track_OBJ(const std::string& filename) {
    auto mesh = std::make_shared<Mesh>();
    generate_track_mesh(mesh);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    
    file << "# Racetrack exported from editor\n";
    file << "# Vertices: " << mesh->verts.size() << "\n\n";

    file << "mtllib exported_track.mtl\n";
    file << "o track\n";
    // Write vertices
    for (const auto& vertex : mesh->verts) {
        file << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
    }
    
    // Write texture coordinates
    for (const auto& uv : mesh->mappings) {
        file << "vt " << uv.x << " " << uv.y << "\n";
    }
    
    // Write normals
    for (const auto& normal : mesh->normals) {
        file << "vn " << normal.x << " " << normal.y << " " << normal.z << "\n";
    }
    
    // Write faces
    if (!mesh->groups.empty() && !mesh->groups[0]->faces.empty()) {
        file << "\nusemtl TrackMaterial\n";
        file << "s 1\n"; // Smoothing group
        
        for (const auto& face : mesh->groups[0]->faces) {
            file << "f";
            for (size_t j = 0; j < face->verts.size(); j++) {
                // OBJ indices are 1-based
                file << " " << (face->verts[j] + 1) << "/" 
                     << (face->textures[j] + 1) << "/" 
                     << (face->normals[j] + 1);
            }
            file << "\n";
        }
    }
    
    file.close();
    std::cout << "Exported track to " << filename << std::endl;
}

void TrackEditor::export_animation_file(const std::string& filename) {
    auto centerPoints = center_spline.evaluateCurve();
    
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