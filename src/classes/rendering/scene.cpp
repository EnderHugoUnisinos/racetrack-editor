#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "scene.h"

void Scene::add_object(std::shared_ptr<Obj3D> object){
    objects.push_back(object);
}
void Scene::remove_object(int index) { 
    if (index >= 0 && index < objects.size()) {
        objects.erase(objects.begin() + index);
    }
}
void Scene::update(float delta_time){}
void Scene::cleanup(){};
void Scene::unload_scene(){};