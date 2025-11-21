#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#include "obj3d.h"

void Obj3D::update(float deltaTime) {
    if (is_animated && animation) {
        animation_time += deltaTime;
        glm::vec3 newPos = animation->get_position_at_time(animation_time);
        glm::vec3 nextPos = animation->get_position_at_time(animation_time + deltaTime);
        glm::vec3 direction = nextPos - newPos;
        direction = glm::normalize(direction);
        float angle = atan2(direction.x, direction.z);


        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::translate(glm::mat4(1.0f), newPos) * rotation;
    }
}

void process_vertex_for_group(const glm::ivec3& indices, 
    const std::vector<glm::vec3>& vs,
    const std::vector<glm::vec2>& ms,
    const std::vector<glm::vec3>& ns,
    std::vector<glm::vec3>& out_verts,
    std::vector<glm::vec2>& out_mappings,
    std::vector<glm::vec3>& out_normals) {
    
    glm::vec3 v(0.0f);
    glm::vec2 m(0.0f);
    glm::vec3 n(0.0f, 1.0f, 0.0f);

    if (indices.x >= 1) v = vs[indices.x - 1];
    if (indices.y >= 1) m = ms[indices.y - 1];
    if (indices.z >= 1) n = ns[indices.z - 1];
    
    out_verts.push_back(v);
    out_mappings.push_back(m);
    out_normals.push_back(n);
}

void Obj3D::setup_buffers(){
    if (buffers_created) return;

    for (auto group : mesh->groups) {
        // Process this group's data separately
        std::vector<glm::vec3> group_verts;
        std::vector<glm::vec2> group_mappings;
        std::vector<glm::vec3> group_normals;
        
        for (const auto& face : group->faces) {
            if (face->verts.size() > 3) {
                for (size_t i = 1; i < face->verts.size() - 1; i++) {
                    process_vertex_for_group(glm::ivec3(face->verts[0], face->textures[0], face->normals[0]), 
                                           mesh->verts, mesh->mappings, mesh->normals,
                                           group_verts, group_mappings, group_normals);
                    process_vertex_for_group(glm::ivec3(face->verts[i], face->textures[i], face->normals[i]), 
                                           mesh->verts, mesh->mappings, mesh->normals,
                                           group_verts, group_mappings, group_normals);
                    process_vertex_for_group(glm::ivec3(face->verts[i + 1], face->textures[i + 1], face->normals[i + 1]), 
                                           mesh->verts, mesh->mappings, mesh->normals,
                                           group_verts, group_mappings, group_normals);
                }
            } else {
                for (size_t i = 0; i < face->verts.size(); i++) {
                    process_vertex_for_group(glm::ivec3(face->verts[i], face->textures[i], face->normals[i]), 
                                           mesh->verts, mesh->mappings, mesh->normals,
                                           group_verts, group_mappings, group_normals);
                }
            }
        }
        
        group->vert_count = group_verts.size();
        
        std::vector<float> interleaved_data;
        for (size_t i = 0; i < group_verts.size(); i++) {
            // Position
            interleaved_data.push_back(group_verts[i].x);
            interleaved_data.push_back(group_verts[i].y);
            interleaved_data.push_back(group_verts[i].z);

            // Texture coordinates
            if (i < group_mappings.size()) {
                interleaved_data.push_back(group_mappings[i].x);
                interleaved_data.push_back(group_mappings[i].y);
            } else {
                interleaved_data.push_back(0.0f);
                interleaved_data.push_back(0.0f);
            }

            // Normals
            if (i < group_normals.size()) {
                interleaved_data.push_back(group_normals[i].x);
                interleaved_data.push_back(group_normals[i].y);
                interleaved_data.push_back(group_normals[i].z);
            } else {
                interleaved_data.push_back(0.0f);
                interleaved_data.push_back(1.0f);
                interleaved_data.push_back(0.0f);
            }
        }

        glGenVertexArrays(1, &group->VAO);
        glGenBuffers(1, &group->VBO);

        glBindVertexArray(group->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, group->VBO);
        glBufferData(GL_ARRAY_BUFFER, interleaved_data.size() * sizeof(float), 
                     interleaved_data.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                             (void*)(3 * sizeof(float)));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                             (void*)(5 * sizeof(float)));

        glBindVertexArray(0);
    }
    buffers_created = true;
}

void Obj3D::calculate_bbox(){
    if (mesh->groups.empty()) return;

    glm::vec3 min(FLT_MAX), max(FLT_MIN);
    
    for (const auto& pos : mesh->verts) {
        min = glm::min(min, pos);
        max = glm::max(max, pos);
    }
    
    bbox->min = min;
    bbox->max = max;
    bbox->center = (min + max) * 0.5f;
    bbox->size = max - min;
}

bool Obj3D::check_collision(const glm::vec3& point) const {
    if (!collidable) return false;
    
    glm::vec3 world_point = glm::vec3(glm::inverse(transform) * glm::vec4(point, 1.0f));
    
    return (world_point.x >= bbox->min.x && world_point.x <= bbox->max.x &&
            world_point.y >= bbox->min.y && world_point.y <= bbox->max.y &&
            world_point.z >= bbox->min.z && world_point.z <= bbox->max.z);
}