#pragma once

#include "bullet.h"
#include "../rendering/3D/obj3d.h"

class BulletManager {
public:
    std::vector<std::shared_ptr<Bullet>> bullets;
    GLuint cubeVAO, cubeVBO;
    bool buffersInitialized;

    BulletManager() : buffersInitialized(false), cubeVAO(0), cubeVBO(0) {}

    void init() {
        setup_cube_buffers();
    }
    void addBullet(const glm::vec3& position, const glm::vec3& direction);
    void update(float deltaTime);
    void checkCollisions(std::vector<std::shared_ptr<Obj3D>>& objects);
    std::shared_ptr<BoundingBox> transformBoundingBox(const std::shared_ptr<BoundingBox>& bbox, const glm::mat4& transform);
    bool checkAABBCollision(const std::shared_ptr<BoundingBox>& a, const std::shared_ptr<BoundingBox>& b) const;
    glm::vec3 calculateCollisionNormal(const std::shared_ptr<BoundingBox>& bulletBox, const std::shared_ptr<BoundingBox>& objBox) const;
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
        buffersInitialized = false;
    }
};