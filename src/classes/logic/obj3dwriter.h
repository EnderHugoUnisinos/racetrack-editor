#pragma once

#include <vector>
#include <string>

#include "../rendering/3D/obj3d.h"

class Obj3DWriter{
public:
    static void write(std::shared_ptr<Obj3D> obj);
    static std::shared_ptr<Mesh> load_from_file(const std::string& filename);
    static std::vector<std::shared_ptr<Obj3D>> file_reader();
    static std::unordered_map<std::string, std::shared_ptr<Material>> load_materials(const std::string& mtlFilePath);
    static std::string find_texture_file(const std::string& directory, const std::string& baseName);
};