#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "mesh.h"

void Mesh::process_data(){
    processed_verts.clear();
    processed_mappings.clear();
    processed_normals.clear();
    
    for (auto group : groups) {
        for (const auto& face : group->faces) {
            if (face->verts.size() > 3) {
                for (size_t i = 1; i < face->verts.size() - 1; i++) {
                    process_vertex(glm::ivec3(face->verts[0], face->textures[0], face->normals[0]), verts, mappings, normals);
                    process_vertex(glm::ivec3(face->verts[i], face->textures[i], face->normals[i]), verts, mappings, normals);
                    process_vertex(glm::ivec3(face->verts[i + 1], face->textures[i + 1], face->normals[i + 1]), verts, mappings, normals);
                }
            } else {
                for (size_t i = 0; i < face->verts.size(); i++) {
                    process_vertex(glm::ivec3(face->verts[i], face->textures[i], face->normals[i]), verts, mappings, normals);
                }
            }
        }
    }
}


void Mesh::process_vertex(const glm::ivec3& indices, 
    std::vector<glm::vec3>& vs,
    std::vector<glm::vec2>& ms,
    std::vector<glm::vec3>& ns
){
    glm::vec3 v, n;
    glm::vec2 m;

    if (indices.x >= 1) v = vs[indices.x - 1];
    if (indices.y >= 1) m = ms[indices.y - 1];
    if (indices.z >= 1) n = ns[indices.z - 1];
    
    processed_verts.push_back(v);
    processed_mappings.push_back(m);
    processed_normals.push_back(n);
}