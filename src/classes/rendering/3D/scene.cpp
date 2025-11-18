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
void Scene::update(float delta_time){
    for (auto obj : objects) {
        if (obj) {
            obj->update(delta_time);
        }
    }
}
void Scene::cleanup() {
    for (auto obj : objects) {
        if (obj && obj->mesh) {
            for (auto group : obj->mesh->groups) {
                if (group) {
                    if (group->material) {
                        group->material->cleanup();
                    }
                    if (group->VAO) {
                        glDeleteVertexArrays(1, &group->VAO);
                        group->VAO = 0;
                    }
                    if (group->VBO) {
                        glDeleteBuffers(1, &group->VBO);
                        group->VBO = 0;
                    }
                }
            }
        }
    }
    objects.clear();
}