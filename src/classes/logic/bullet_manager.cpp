#include "bullet_manager.h"

void BulletManager::addBullet(const glm::vec3& position, const glm::vec3& direction) {
    bullets.push_back(std::make_shared<Bullet>(position, direction));
}

void BulletManager::update(float deltaTime) {
    for (auto& bullet : bullets) {
            bullet->update(deltaTime);
        }
        
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const std::shared_ptr<Bullet>& b) { return !b->active; }),
    bullets.end());
}

void BulletManager::checkCollisions(std::vector<std::shared_ptr<Obj3D>>& objects) {
    for (auto& bullet : bullets) {
        if (!bullet->active) continue;
        
        for (auto& obj : objects) {
            if (!obj->collidable) continue;
            
            std::shared_ptr<BoundingBox> bulletBBox = bullet->get_world_bbox();
            
            std::shared_ptr<BoundingBox> objBBox = obj->bbox;
            glm::mat4 objTransform = obj->transform;
            
            std::shared_ptr<BoundingBox> worldObjBBox = transformBoundingBox(objBBox, objTransform);
            
            if (checkAABBCollision(bulletBBox, worldObjBBox)) {
                if (obj->eliminable) {
                    obj->collidable = false;
                    obj->active = false;
                    bullet->active = false;
                } else {
                    glm::vec3 normal = calculateCollisionNormal(bulletBBox, worldObjBBox);
                    bullet->reflect(normal);
                }
            }
        }
    }
}

std::shared_ptr<BoundingBox> BulletManager::transformBoundingBox(const std::shared_ptr<BoundingBox>& bbox, const glm::mat4& transform) {
    std::shared_ptr<BoundingBox> result = std::make_shared<BoundingBox>();
        
    glm::vec3 corners[8] = {
        glm::vec3(bbox->min.x, bbox->min.y, bbox->min.z),
        glm::vec3(bbox->max.x, bbox->min.y, bbox->min.z),
        glm::vec3(bbox->min.x, bbox->max.y, bbox->min.z),
        glm::vec3(bbox->max.x, bbox->max.y, bbox->min.z),
        glm::vec3(bbox->min.x, bbox->min.y, bbox->max.z),
        glm::vec3(bbox->max.x, bbox->min.y, bbox->max.z),
        glm::vec3(bbox->min.x, bbox->max.y, bbox->max.z),
        glm::vec3(bbox->max.x, bbox->max.y, bbox->max.z)
    };
    
    glm::vec3 transformed = glm::vec3(transform * glm::vec4(corners[0], 1.0f));
    result->min = transformed;
    result->max = transformed;
    
    for (int i = 1; i < 8; i++) {
        transformed = glm::vec3(transform * glm::vec4(corners[i], 1.0f));
        result->min = glm::min(result->min, transformed);
        result->max = glm::max(result->max, transformed);
    }
    
    result->center = (result->min + result->max) * 0.5f;
    result->size = result->max - result->min;
    result->halfSize = result->size * 0.5f;
    
    return result;
}

bool BulletManager::checkAABBCollision(const std::shared_ptr<BoundingBox>& a, const std::shared_ptr<BoundingBox>& b) const {
    return (a->min.x <= b->max.x && a->max.x >= b->min.x) &&
    (a->min.y <= b->max.y && a->max.y >= b->min.y) &&
    (a->min.z <= b->max.z && a->max.z >= b->min.z);
}

glm::vec3 BulletManager::calculateCollisionNormal(const std::shared_ptr<BoundingBox>& bulletBox, const std::shared_ptr<BoundingBox>& objBox) const {
    glm::vec3 penetration;
    const float epsilon = 0.001f;
    
    penetration.x = std::min(bulletBox->max.x - objBox->min.x, objBox->max.x - bulletBox->min.x) + epsilon;
    penetration.y = std::min(bulletBox->max.y - objBox->min.y, objBox->max.y - bulletBox->min.y) + epsilon;
    penetration.z = std::min(bulletBox->max.z - objBox->min.z, objBox->max.z - bulletBox->min.z) + epsilon;
    
    if (penetration.x <= penetration.y && penetration.x <= penetration.z) {
        return glm::vec3(glm::sign(bulletBox->center.x - objBox->center.x), 0.0f, 0.0f);
    } else if (penetration.y <= penetration.x && penetration.y <= penetration.z) {
        return glm::vec3(0.0f, glm::sign(bulletBox->center.y - objBox->center.y), 0.0f);
    } else {
        return glm::vec3(0.0f, 0.0f, glm::sign(bulletBox->center.z - objBox->center.z));
    }
}

void BulletManager::setup_cube_buffers() {
    if (buffersInitialized) return;
        
    float vertices[] = {
        // Positions          // Normals
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
    buffersInitialized = true;
}

void BulletManager::render(GLuint shaderProgram) {
    if (!buffersInitialized) return;
        
    glBindVertexArray(cubeVAO);
    
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), 0);

    for (const auto& bullet : bullets) {
        if (bullet->active) {
            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, bullet->position);
            model = glm::scale(model, glm::vec3(0.2f));
            
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            
            if (bullet->reflected) {
                glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuseColor"), 1.0f, 0.5f, 0.0f);
            } else {
                glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuseColor"), 1.0f, 1.0f, 1.0f);
            }

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }
    
    glBindVertexArray(0);
}