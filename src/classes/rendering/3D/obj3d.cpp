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

    for (auto group : mesh->groups) {
        std::vector<float> interleaved_data;

        // Use the processed vertex data directly
        for (size_t i = 0; i < mesh->processed_verts.size(); i++) {
            // Position
            interleaved_data.push_back(mesh->processed_verts[i].x);
            interleaved_data.push_back(mesh->processed_verts[i].y);
            interleaved_data.push_back(mesh->processed_verts[i].z);

            // Texture coordinates (if available)
            if (i < mesh->processed_mappings.size()) {
                interleaved_data.push_back(mesh->processed_mappings[i].x);
                interleaved_data.push_back(mesh->processed_mappings[i].y);
            } else {
                interleaved_data.push_back(0.0f);
                interleaved_data.push_back(0.0f);
            }

            // Normals (if available)
            if (i < mesh->processed_normals.size()) {
                interleaved_data.push_back(mesh->processed_normals[i].x);
                interleaved_data.push_back(mesh->processed_normals[i].y);
                interleaved_data.push_back(mesh->processed_normals[i].z);
            } else {
                interleaved_data.push_back(0.0f);
                interleaved_data.push_back(1.0f);
                interleaved_data.push_back(0.0f);
            }
        }

        // Generate VAO and VBO
        glGenVertexArrays(1, &group->VAO);
        glGenBuffers(1, &group->VBO);

        glBindVertexArray(group->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, group->VBO);
        glBufferData(GL_ARRAY_BUFFER, interleaved_data.size() * sizeof(float), 
                     interleaved_data.data(), GL_STATIC_DRAW);

        // Set up vertex attributes
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        
        // Texture coordinates  
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                             (void*)(3 * sizeof(float)));
        
        // Normals
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 
                             (void*)(5 * sizeof(float)));

        glBindVertexArray(0);
        
        group->vert_count = mesh->processed_verts.size();
    }
    buffers_created = true;
};