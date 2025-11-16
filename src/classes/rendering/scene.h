#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "obj3d.h"

class Scene{
public:
    std::vector<std::shared_ptr<Obj3D>> objects;

    void add_object(std::shared_ptr<Obj3D> object);
    void remove_object(int index);
    
    void update(float delta_time);

    void render(GLuint& shaderProgram) const;
    void cleanup();
    void unload_scene();
};
