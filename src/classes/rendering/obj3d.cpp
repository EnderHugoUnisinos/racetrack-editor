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

void Obj3D::setup_buffers(){
    if (buffers_created) return;

    for (auto *group : mesh->groups) {
        std::vector<float> interleaved_data;

        for (auto& face : group->faces){
            int n;
            for (int i = 0; i < face->verts.size(); i++){
                glm::vec3& v = mesh->verts[face->verts[i]];
                interleaved_data.push_back(v.x);
                interleaved_data.push_back(v.y);
                interleaved_data.push_back(v.z);

                glm::vec2& vt = mesh->mappings[face->textures[i]];
                interleaved_data.push_back(vt.x);
                interleaved_data.push_back(vt.y);
                glm::vec3& vn = mesh->normals[face->normals[i]];
                interleaved_data.push_back(vn.x);
                interleaved_data.push_back(vn.y);
                interleaved_data.push_back(vn.z);
                n++;
            }
        }

       // Generate VAO and single VBO for interleaved data
        glGenVertexArrays(1, &group->VAO);
        glGenBuffers(1, &group->VBO);

        glBindVertexArray(group->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, group->VBO);
        glBufferData(GL_ARRAY_BUFFER, interleaved_data.size() * sizeof(float), 
                     interleaved_data.data(), GL_STATIC_DRAW);

        // Set up vertex attributes with strides
        // Position (attribute 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        
        // Texture coordinates (attribute 1)  
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                             (void*)(3 * sizeof(float)));
        
        // Normals (attribute 2)
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                             (void*)(5 * sizeof(float)));

        glBindVertexArray(0);
        
        group->vert_count = interleaved_data.size() / 8; // 8 floats per vertex
    }
    buffers_created = true;
};