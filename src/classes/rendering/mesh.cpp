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
    std::vector<glm::vec3> vs = verts;
    std::vector<glm::vec2> ms = mappings;
    std::vector<glm::vec3> ns = normals;
    
    // Clear and rebuild arrays
    mappings.clear();
    verts.clear();
    normals.clear();
    
    for (auto group : groups) {
        for (const auto& face : group->faces) {
            // Triangulate faces with more than 3 vertices using fan triangulation
            if (face->verts.size() > 3) {
                for (size_t i = 1; i < face->verts.size() - 1; i++) {
                    process_vertex(glm::ivec3(face->verts[0], face->textures[0], face->normals[0]), vs, ms, ns);
                    process_vertex(glm::ivec3(face->verts[i], face->textures[i], face->normals[i]), vs, ms, ns);
                    process_vertex(glm::ivec3(face->verts[i + 1], face->textures[i + 1], face->normals[i + 1]), vs, ms, ns);
                }
            } else {
                // For triangles, process all vertices
                for (size_t i = 0; i < face->verts.size(); i++) {
                    process_vertex(glm::ivec3(face->verts[i], face->textures[i], face->normals[i]), vs, ms, ns);
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
    
    verts.push_back(v);
    mappings.push_back(m);
    normals.push_back(n);
}