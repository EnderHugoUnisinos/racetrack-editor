#pragma once

#include "bullet.h"
#include "../rendering/3D/obj3d.h"

class BulletManager {
public:
    std::vector<std::shared_ptr<Bullet>> bullets;
    GLuint cubeVAO, cubeVBO;
    bool buffers_initialized;

    BulletManager() : buffers_initialized(false), cubeVAO(0), cubeVBO(0) {}

    void init() {
        setup_cube_buffers();
    }
    void add_bullet(const glm::vec3& position, const glm::vec3& direction);
    void update(float deltaTime);
    void checkCollisions(std::vector<std::shared_ptr<Obj3D>>& objects);
    std::shared_ptr<BoundingBox> transform_bounding_box(const std::shared_ptr<BoundingBox>& bbox, const glm::mat4& transform);
    bool check_AABB_collision(const std::shared_ptr<BoundingBox>& a, const std::shared_ptr<BoundingBox>& b) const;
    glm::vec3 calculate_collision_normal(const std::shared_ptr<BoundingBox>& bulletBox, const std::shared_ptr<BoundingBox>& objBox) const;
    void setup_cube_buffers();
    void render(GLuint shaderProgram);
    void cleanup() {
        if (cubeVAO) {
            glDeleteVertexArrays(1, &cubeVAO);
            cubeVAO = 0;
        }
        if (cubeVBO) {
            glDeleteBuffers(1, &cubeVBO);
            cubeVBO = 0;
        }
        buffers_initialized = false;
    }
};