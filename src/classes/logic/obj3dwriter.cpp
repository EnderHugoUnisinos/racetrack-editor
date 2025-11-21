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
#include <unordered_map>

#include "obj3dwriter.h"

std::string Obj3DWriter::find_texture_file(const std::string& directory, const std::string& baseName) {
    std::vector<std::string> extensions = {".png", ".jpg", ".jpeg", ".bmp", ".tga", ".tiff"};
    
    for (const auto& ext : extensions) {
        std::string fullPath = directory + "/" + baseName + ext;
        if (std::filesystem::exists(fullPath)) {
            return baseName + ext;
        }
    }
    
    std::string fullPath = directory + "/" + baseName;
    if (std::filesystem::exists(fullPath)) {
        return baseName;
    }
    
    return baseName;
}

void Obj3DWriter::write(std::shared_ptr<Obj3D> obj){
    std::shared_ptr<Mesh> mesh = load_from_file(obj->obj_file);
    obj->mesh = mesh;
    obj->setup_buffers();
};

std::shared_ptr<Mesh> Obj3DWriter::load_from_file(const std::string& filename){
    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return nullptr;
    }

    size_t last_slash = filename.find_last_of("/\\");
    std::string directory = (last_slash == std::string::npos) ? "" : filename.substr(0, last_slash);

    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::shared_ptr<Group> current_group = nullptr;
    std::string current_material_name;
    
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "g" || prefix == "o") {
            std::string name;
            iss >> name;
            if (name.empty() && prefix == "o") {
                name = "Default";
            }
            
            current_group = std::make_shared<Group>(name);
            mesh->groups.push_back(current_group);
            current_material_name.clear();
        }
        else if (prefix == "mtllib") {
            std::string mtl_file;
            iss >> mtl_file;
            std::string full_mtl_path = directory + "/" + mtl_file;
            auto loaded_materials = load_materials(full_mtl_path);
            materials.insert(loaded_materials.begin(), loaded_materials.end());
        }
        else if (prefix == "usemtl") {
            if (!current_group) {
                current_group = std::make_shared<Group>("Default");
                mesh->groups.push_back(current_group);
            }
            
            iss >> current_material_name;
            if (materials.find(current_material_name) != materials.end()) {
                current_group->material = materials[current_material_name];
            } else {
                std::cerr << "Warning: Material '" << current_material_name << "' not found" << std::endl;
            }
        }
        else if (prefix == "v") {
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            mesh->verts.push_back(position);
        }
        else if (prefix == "vt") {
            glm::vec2 texCoord;
            iss >> texCoord.x >> texCoord.y;
            mesh->mappings.push_back(texCoord);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            mesh->normals.push_back(normal);
        }
        else if (prefix == "f") {
            if (!current_group) {
                current_group = std::make_shared<Group>("Default");
                mesh->groups.push_back(current_group);
            }
            
            std::shared_ptr<Face> face = std::make_shared<Face>();
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
            
            if (face->verts.size() >= 3) {
                current_group->faces.push_back(face);
            }
        }
    }

    mesh->process_data();
    file.close();
    return mesh;
}

std::vector<std::shared_ptr<Obj3D>> Obj3DWriter::file_reader() {
    std::string path = "../objs/config.cfg";
    std::vector<std::shared_ptr<Obj3D>> objs;
    
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
        int selectable;
        int collidable;
        int eliminable;
        
        if (iss >> mesh_nm >> mesh_file >> selectable >> collidable >> eliminable) {
            auto obj3d = std::make_shared<Obj3D>();
            obj3d->name = mesh_nm;
            obj3d->obj_file = mesh_file;
            obj3d->selectable = selectable;
            obj3d->collidable = collidable;
            obj3d->eliminable = eliminable;
            objs.push_back(obj3d);
        }
    }
    return objs;
}

std::unordered_map<std::string, std::shared_ptr<Material>> Obj3DWriter::load_materials(const std::string& mtlFilePath) {
    std::unordered_map<std::string, std::shared_ptr<Material>> materials;
    std::ifstream file(mtlFilePath);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open MTL file " << mtlFilePath << std::endl;
        return materials;
    }
    
    size_t last_slash = mtlFilePath.find_last_of("/\\");
    std::string directory = (last_slash == std::string::npos) ? "" : mtlFilePath.substr(0, last_slash);
    
    std::shared_ptr<Material> current_material;
    std::string line;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        
        if (prefix == "newmtl") {
            std::string name;
            iss >> name;
            current_material = std::make_shared<Material>();
            current_material->name = name;
            materials[name] = current_material;
        }
        else if (prefix == "Ka" && current_material) {
            iss >> current_material->ambient.r >> current_material->ambient.g >> current_material->ambient.b;
        }
        else if (prefix == "Kd" && current_material) {
            iss >> current_material->diffuse.r >> current_material->diffuse.g >> current_material->diffuse.b;
        }
        else if (prefix == "Ks" && current_material) {
            iss >> current_material->specular.r >> current_material->specular.g >> current_material->specular.b;
        }
        else if (prefix == "Ns" && current_material) {
            iss >> current_material->shininess;
        }
        else if (prefix == "map_Kd" && current_material) {
            std::string texture_path;
            iss >> texture_path;
            texture_path.erase(std::remove(texture_path.begin(), texture_path.end(), '\r'), texture_path.end());
            current_material->diffuseMap = find_texture_file(directory, texture_path);
            //std::cout << directory << " " << texture_path << std::endl;
            std::cout << "Set diffuse map: " << current_material->diffuseMap << std::endl;
        }
        else if (prefix == "map_Ks" && current_material) {
            std::string texture_path;
            iss >> texture_path;
            texture_path.erase(std::remove(texture_path.begin(), texture_path.end(), '\r'), texture_path.end());
            current_material->specularMap = find_texture_file(directory, texture_path);
        }
        else if (prefix == "map_Bump" && current_material) {
            std::string texture_path;
            iss >> texture_path;
            texture_path.erase(std::remove(texture_path.begin(), texture_path.end(), '\r'), texture_path.end());
            current_material->normalMap = Obj3DWriter::find_texture_file(directory, texture_path);
        }
    }
    
    file.close();
    return materials;
}