#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "obj3dwriter.h"

void Obj3DWriter::write(Obj3D*obj){
    Mesh* mesh = load_from_file(obj->file);
    obj->mesh = mesh;
    obj->setup_buffers();
};

Mesh* Obj3DWriter::load_from_file(const std::string& filename){
    
    Mesh* mesh = new Mesh;
    std::ifstream file(filename);
    mesh->groups.push_back(new Group("Default"));
    Group* group = mesh->groups[0];

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return mesh;
    }

            size_t last_slash = filename.find_last_of("/\\");
    std::string filename_with_ext = (last_slash == std::string::npos) 
    ? filename 
    : filename.substr(last_slash + 1);

    /* OBJ3D WRITER
    std::string directory = get_directory(filename);
    std::string configPath = directory + "mesh_config.txt";
    */
    
    std::string line;
    std::string currentGroup = "default";
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "g" || prefix == "o") {
            // group
            std::string name;
            iss >> name;
            if (!name.empty()) {
                currentGroup = name;
                mesh->groups.push_back(new Group(currentGroup));
                group = mesh->groups[mesh->groups.size() - 1];
            }
        }
        else if (prefix == "v") {
            // vertex
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            if (group) mesh->verts.push_back(position);
        }
        else if (prefix == "vt") {
            // texture coord
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            if (group) mesh->mappings.push_back(texCoord);
        }
        else if (prefix == "vn") {
            // normal
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            if (group) mesh->normals.push_back(normal);
        }
        else if (prefix == "f") {
            // face
            Face* face = new Face;
            std::string vertexData;
            
            while (iss >> vertexData) {
                glm::ivec3 indices(-1, -1, -1);
                std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
                std::istringstream viss(vertexData);
                
                viss >> indices.x;
                if (viss.peek() != EOF) viss >> indices.y;
                if (viss.peek() != EOF) viss >> indices.z;
                
                face->verts.push_back(indices[0]);
                face->textures.push_back(indices[1]);
                face->normals.push_back(indices[2]);
            }
            
            if (group && face->verts.size() >= 3) {
                group->faces.push_back(face);
            }
        }
    }
    
    mesh->process_data();
    file.close();
    return mesh;
}


std::vector<Obj3D*> Obj3DWriter::file_reader() {
    std::string path = "../objs/config.cfg";
    std::vector<Obj3D*> objs;
    
    std::ifstream arq(path);
    if (!arq.is_open()) {
        std::cerr << "Warning: Could not open config file: " << path << std::endl;
        return objs;
    }

    std::string line;
    while (std::getline(arq, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string mesh_nm;
        std::string mesh_file;
        
        if (iss >> mesh_nm >> mesh_file) {
            Obj3D *obj3d = new Obj3D;
            obj3d->name = mesh_nm;
            obj3d->file = mesh_file;
            objs.push_back(obj3d);
        }
    }
    return objs;
};
