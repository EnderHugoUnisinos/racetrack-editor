#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "classes/logic/track_editor.h"
#include "classes/rendering/3D/scene.h"
#include "classes/logic/obj3dwriter.h"
#include "classes/logic/bullet_manager.h"

// STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace std;
namespace fs = std::filesystem;

// Dimensões da janela
const GLuint WIDTH = 1200, HEIGHT = 800;

// Variáveis globais de controle da câmera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 20.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float fov = 45.0f;

// Controle de tempo entre frames
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// IDs de shader
GLuint shaderID;

GLuint VAO;

GLFWwindow* window;

#pragma region Shaders
const GLchar* vertexShaderSource = R"glsl(
    #version 450 core
    layout (location = 0) in vec3 position;
    layout (location = 1) in vec2 texCoord;
    layout (location = 2) in vec3 normal;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    out vec3 FragPos;
    out vec2 TexCoord;
    out vec3 Normal;
    
    void main()
    {
        FragPos = vec3(model * vec4(position, 1.0));
        TexCoord = texCoord;
        Normal = mat3(transpose(inverse(model))) * normal;
        gl_Position = projection * view * vec4(FragPos, 1.0);
    }
)glsl";

const GLchar* fragmentShaderSource = R"glsl(
    #version 450 core
    in vec3 FragPos;
    in vec2 TexCoord;
    in vec3 Normal;
    out vec4 FragColor;
    
    struct Material {
        sampler2D diffuse;
        sampler2D specular;
        float shininess;
        vec3 ambient;
        vec3 diffuseColor;
        vec3 specularColor;
    };
    
    struct Light {
        vec3 position;
        vec3 ambient;
        vec3 diffuse;
        vec3 specular;
    };
    
    uniform Material material;
    uniform Light light;
    uniform vec3 viewPos;
    uniform bool useDiffuseTexture;
    uniform bool useSpecularTexture;
    
    void main()
    {
        vec3 ambient = light.ambient * material.ambient;
        
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse;
        
        if(useDiffuseTexture) {
            diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
        } else {
            diffuse = light.diffuse * diff * material.diffuseColor;
        }
        
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular;
        
        if(useSpecularTexture) {
            specular = light.specular * spec * texture(material.specular, TexCoord).rgb;
        } else {
            specular = light.specular * spec * material.specularColor;
        }
        
        vec3 result = ambient + diffuse + specular;
        FragColor = vec4(result, 1.0);
    }
)glsl";
#pragma endregion

std::unique_ptr<Scene> current_scene = std::make_unique<Scene>();
std::unique_ptr<BulletManager> bullet_manager = std::make_unique<BulletManager>();
int currentObjectIndex;

std::unique_ptr<TrackEditor> trackEditor = std::make_unique<TrackEditor>();
std::shared_ptr<Obj3D> current_track = std::make_shared<Obj3D>();
std::shared_ptr<Obj3D> loaded_track = std::make_shared<Obj3D>();
std::shared_ptr<Obj3D> racecar = std::make_shared<Obj3D>();

// 0 = track editor / 1 = 3D model viewer
int current_mode = 0;

int selected_object = 0;

static GLuint pointShader = 0;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void first_person(double xpos, double ypos){
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.05f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void update_track_preview() {
    if (std::find(current_scene->objects.begin(), current_scene->objects.end(), current_track) == current_scene->objects.end()) {
        current_track->name = "Track";
        current_scene->add_object(current_track);
        std::cout << "Added track to scene" << std::endl;
    }
    if (!current_track->mesh) {
        current_track->mesh = std::make_shared<Mesh>();
    }
    trackEditor->generate_track_mesh(current_track->mesh);
    current_track->buffers_created = false;
    current_track->obj_file = "../objs/track/...";
    current_track->setup_buffers();

    auto it = std::find(current_scene->objects.begin(), current_scene->objects.end(), current_track);
    if (it == current_scene->objects.end()) {
        current_scene->add_object(current_track);
        std::cout << "Added track to scene" << std::endl;
    }
}

void track_editor(double xpos, double ypos) {
    // Store mouse position for click handling
    static double lastClickX = 0, lastClickY = 0;
    lastClickX = xpos;
    lastClickY = ypos;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (current_mode == 0 && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        
        // Convert screen coordinates to normalized device coordinates
        float ndcX = (2.0f * xpos) / WIDTH - 1.0f;
        float ndcY = 1.0f - (2.0f * ypos) / HEIGHT;
        
        // Create a ray in world coordinates using inverse matrices
        glm::vec4 rayClip = glm::vec4(ndcX, ndcY, -1.0f, 1.0f);
        
        // Get projection and view matrices
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        
        // Convert to eye coordinates
        glm::mat4 invProjection = glm::inverse(projection);
        glm::vec4 rayEye = invProjection * rayClip;
        rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
        
        // Convert to world coordinates
        glm::mat4 invView = glm::inverse(view);
        glm::vec4 rayWorld = invView * rayEye;
        glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld));
        
        // Calculate intersection with ground plane (y = 0)
        // Ray equation: P = cameraPos + t * rayDir
        // We want Py = 0, so: cameraPos.y + t * rayDir.y = 0
        if (fabs(rayDir.y) > 0.0001f) {
            float t = -cameraPos.y / rayDir.y;
            glm::vec3 worldPos = cameraPos + t * rayDir;
            
            // Add control point at ground level
            trackEditor->add_control_point(glm::vec2(worldPos.x, worldPos.z));
            std::cout << "Added control point at: " << worldPos.x << ", " << worldPos.z << std::endl;
            
            if (trackEditor->control_points.size() >= 4) {
                update_track_preview();
            }
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (current_mode == 0) {
        track_editor(xpos, ypos);
    }
    else if (current_mode == 1)
    {
        first_person(xpos, ypos);
    }
}

void finish_track(){
    trackEditor->export_track_OBJ("../objs/track/exported_track.obj");
    trackEditor->export_animation_file("../objs/track/animation_path.txt");
    trackEditor->clear_control_points();
    if (current_track) {
        auto it = std::find(current_scene->objects.begin(), current_scene->objects.end(), current_track);
        if (it != current_scene->objects.end()) {
            current_scene->objects.erase(it);
        }
    }
    loaded_track->buffers_created = 0;
    Obj3DWriter::write(loaded_track);
    racecar->animation->keyframes.clear();
    racecar->animation->load_from_file("../objs/track/animation_path.txt");
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_TAB) {
            // Switch between modes
            current_mode = (current_mode + 1) % 2;
            if (current_mode == 0) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
            }
        }
        else if (key == GLFW_KEY_E && current_mode == 0) {
            // Export track
            trackEditor->export_track_OBJ("../objs/track/exported_track.obj");
            trackEditor->export_animation_file("../objs/track/animation_path.txt");
            std::cout << "Track exported!" << std::endl;
        }
        else if (key == GLFW_KEY_BACKSPACE && current_mode == 0) {
            //IF BACKSPACE REMOVE LAST CONTROL POINT
            trackEditor->pop_back_control_points();
            update_track_preview();
        }
        else if (key == GLFW_KEY_C && current_mode == 0) {
            // CLEAR POINTS
            trackEditor->clear_control_points();
            if (current_track) {
                auto it = std::find(current_scene->objects.begin(), current_scene->objects.end(), current_track);
                if (it != current_scene->objects.end()) {
                    current_scene->objects.erase(it);
                }
            }
        }
        else if (key == GLFW_KEY_ENTER && current_mode == 0) {
            finish_track();
        }
        else if (key == GLFW_KEY_KP_ADD) {
            int size = current_scene->objects.size();
            currentObjectIndex += 1;
            if (currentObjectIndex >= size) {
                currentObjectIndex = 0;
            }
            if (!current_scene->objects[currentObjectIndex]->selectable) {
                for (int i = currentObjectIndex; i < size; i++){
                    currentObjectIndex = i;
                    if (current_scene->objects[i]->selectable) {
                        break;
                    }
                }
            }
            if (currentObjectIndex >= size) {
                currentObjectIndex = 0;
            }
        }
        else if (key == GLFW_KEY_SPACE && current_mode == 1) {
            bullet_manager->add_bullet(cameraPos, cameraFront);
        }
    }
}

void processInput(GLFWwindow* window) {
    float cameraSpeed = 5.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 2;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        
    if (currentObjectIndex < current_scene->objects.size()){
        if (current_scene->objects[currentObjectIndex]->selectable) {
            if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS) {
                glm::vec3 v(0.0f, 0.0f, -0.1f);
                current_scene->objects[currentObjectIndex]->transform = glm::translate(current_scene->objects[currentObjectIndex]->transform, v);
            }
            if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS) {
                glm::vec3 v(0.1f, 0.0f, 0.0f);
                current_scene->objects[currentObjectIndex]->transform = glm::translate(current_scene->objects[currentObjectIndex]->transform, v);
                
            }
            if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS) {
                glm::vec3 v(-0.1f, 0.0f, 0.0f);
                current_scene->objects[currentObjectIndex]->transform = glm::translate(current_scene->objects[currentObjectIndex]->transform, v);
                
            }
            if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS) {
                glm::vec3 v(0.0f, 0.0f, 0.1f);
                current_scene->objects[currentObjectIndex]->transform = glm::translate(current_scene->objects[currentObjectIndex]->transform, v);
            }
        }
    }
}

void specify_view() {
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    GLuint loc = glGetUniformLocation(shaderID, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
    
    GLuint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");
    glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);
}

void specify_projection() {
    glm::mat4 proj = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    GLuint loc = glGetUniformLocation(shaderID, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
}

GLuint setup_shader() {
    GLint success;
    GLchar infoLog[512];

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        cout << "Erro no Vertex Shader:\n" << infoLog << endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        cout << "Erro no Fragment Shader:\n" << infoLog << endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        cout << "Erro na linkagem do Shader Program:\n" << infoLog << endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void setup_point_buffers(GLuint& VAO, GLuint& VBO) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

GLuint setup_point_shader() {
    const GLchar* pointVertexShader = R"glsl(
        #version 450 core
        layout (location = 0) in vec3 position;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * vec4(position, 1.0);
            gl_PointSize = 12.0;
        }
    )glsl";
    
    const GLchar* pointFragmentShader = R"glsl(
        #version 450 core
        out vec4 FragColor;
        void main() {
            // Draw smooth circular points with border
            vec2 coord = gl_PointCoord - vec2(0.5);
            float dist = length(coord);
            
            if(dist > 0.5)
                discard;
                
            // White fill with black border
            if(dist > 0.4) {
                FragColor = vec4(0.0, 0.0, 0.0, 1.0); // Black border
            } else {
                FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White fill
            }
        }
    )glsl";
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &pointVertexShader, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &pointFragmentShader, nullptr);
    glCompileShader(fragmentShader);
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return shaderProgram;
}

void render_control_points() {
    //if (current_mode != 0) return;
    
    auto controlPoints = trackEditor->control_points;
    if (controlPoints.empty()) return;
    
    // Create a simple shader for points
    static GLuint pointVAO = 0, pointVBO = 0;
    
    // Initialize on first call
    if (pointShader == 0) {
        pointShader = setup_point_shader();
    }
    if (pointVAO == 0) {
        setup_point_buffers(pointVAO, pointVBO);
    }
    
    glUseProgram(pointShader);
    
    // Set matrices for point shader
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)WIDTH / HEIGHT, 0.1f, 100.0f);
    
    glUniformMatrix4fv(glGetUniformLocation(pointShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(pointShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    
    // Update point positions
    std::vector<glm::vec3> pointPositions;
    for (const auto& point : controlPoints) {
        pointPositions.push_back(glm::vec3(point.x, 0.1f, point.y));
    }
    
    glBindVertexArray(pointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    glBufferData(GL_ARRAY_BUFFER, pointPositions.size() * sizeof(glm::vec3), pointPositions.data(), GL_DYNAMIC_DRAW);
    
    // Render points
    glPointSize(8.0f);
    glDrawArrays(GL_POINTS, 0, pointPositions.size());
    glBindVertexArray(0);
}

unsigned int load_texture_from_file(const std::string& filename, const std::string& directory) {
    std::string fullPath = directory + "/" + filename;
    
    // Fix path separators
    std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
    
    std::cout << "Loading texture: " << fullPath << std::endl;
    
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true); // Flip textures if needed
    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        stbi_image_free(data);
        std::cout << "Texture loaded successfully: " << fullPath << " (" << width << "x" << height << ")" << std::endl;
    } else {
        std::cout << "Texture failed to load at path: " << fullPath << std::endl;
        stbi_image_free(data);
        glDeleteTextures(1, &textureID);
        return 0;
    }
    
    return textureID;
}

void setup_default_material(GLuint shaderProgram) {
    glUniform1i(glGetUniformLocation(shaderProgram, "useDiffuseTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "useSpecularTexture"), 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 0.2f, 0.2f, 0.2f);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuseColor"), 0.8f, 0.8f, 0.8f);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.specularColor"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), 32.0f);
}

void setup_material_uniforms(GLuint shaderProgram, const std::shared_ptr<Material>& material, const std::string& textureDirectory) {


    glUniform1i(glGetUniformLocation(shaderProgram, "useDiffuseTexture"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "useSpecularTexture"), 0);
    

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (material->needs_texture_loading()) {
        if (!material->diffuseMap.empty() && material->diffuse_texture == 0) {
            material->diffuse_texture = load_texture_from_file(material->diffuseMap, textureDirectory);
        }
        if (!material->specularMap.empty() && material->specular_texture == 0) {
            material->specular_texture = load_texture_from_file(material->specularMap, textureDirectory);
        }
    } 
    

    glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 
               material->ambient.r, material->ambient.g, material->ambient.b);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuseColor"), 
               material->diffuse.r, material->diffuse.g, material->diffuse.b);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.specularColor"), 
               material->specular.r, material->specular.g, material->specular.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), material->shininess);
    
    if (material->has_diffuse_texture()) {
        glUniform1i(glGetUniformLocation(shaderProgram, "useDiffuseTexture"), 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material->diffuse_texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "material.diffuse"), 0);
    }

    if (material->has_specular_texture()) {
        glUniform1i(glGetUniformLocation(shaderProgram, "useSpecularTexture"), 1);
        glActiveTexture(GL_TEXTURE1); 
        glBindTexture(GL_TEXTURE_2D, material->specular_texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "material.specular"), 1);
    }
}

void setup_light_uniforms(GLuint shaderProgram, glm::vec3 lightPos) {
    // Light properties
    glm::vec3 lightColor = glm::vec3(1.0f);
    glm::vec3 ambientLight = lightColor * 0.1f;
    glm::vec3 diffuseLight = lightColor * 0.8f;
    glm::vec3 specularLight = lightColor * 1.0f;
    
    glUniform3f(glGetUniformLocation(shaderProgram, "light.position"), lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "light.ambient"), ambientLight.r, ambientLight.g, ambientLight.b);
    glUniform3f(glGetUniformLocation(shaderProgram, "light.diffuse"), diffuseLight.r, diffuseLight.g, diffuseLight.b);
    glUniform3f(glGetUniformLocation(shaderProgram, "light.specular"), specularLight.r, specularLight.g, specularLight.b);
    
    // Light attenuation
    glUniform1f(glGetUniformLocation(shaderProgram, "light.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "light.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(shaderProgram, "light.quadratic"), 0.032f);
    
    // Fog settings
    glUniform1i(glGetUniformLocation(shaderProgram, "useFog"), GL_TRUE);
    glm::vec3 fogColor(0.5f, 0.5f, 0.5f);
    glUniform3f(glGetUniformLocation(shaderProgram, "fogColor"), fogColor.r, fogColor.g, fogColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "fogDensity"), 0.0f); // 0 for linear fog
    glUniform1f(glGetUniformLocation(shaderProgram, "fogStart"), 10.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "fogEnd"), 50.0f);
}

void error_log(int cod, const char * description) {
    std::cout << "GLFW Error (" << cod << "): " << description << std::endl;
}

void setup_track(){
    loaded_track->name = "Static Track";
    loaded_track->obj_file = "../objs/track/exported_track.obj";
    //loaded_track->collidable = true;
    current_scene->add_object(loaded_track);
    Obj3DWriter::write(loaded_track);
    
    
    racecar->name = "Racecar";
    racecar->obj_file = "../objs/track/car.obj";
    current_scene->add_object(racecar);
    std::shared_ptr<Animation> animation = std::make_shared<Animation>();
    animation->load_from_file("../objs/track/animation_path.txt");
    racecar->set_animation(animation);
    
    Obj3DWriter::write(racecar);
    
    racecar->collidable = true;
    racecar->calculate_bbox();
   
}

int main() {
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: could not start GLFW3\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Racetrack Editor", NULL, NULL);
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Failure to start GLAD" << endl;
        return -1;
    }

    // Configura callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetErrorCallback(error_log);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderID = setup_shader();

    //STARTUP LOGIC
    //cameraPos = glm::vec3(0.0f, 10.0f, 0.0f); 
    //cameraFront = glm::vec3(0.0f, -50.0f, -1.0f);
    if (pointShader == 0) {
    pointShader = setup_point_shader();  // This should be done once at startup
    }
    current_scene = std::make_unique<Scene>();
    setup_track();
    for (auto obj : Obj3DWriter::file_reader())
    {
        current_scene->add_object(obj);
        Obj3DWriter::write(obj);
        if (obj->collidable) {
            obj->calculate_bbox();
        }
    }

    bullet_manager->init();

    cameraPos = glm::vec3(0.0f, 10.0f, 10.0f); 
    cameraFront = glm::normalize(glm::vec3(0.0f, -0.5f, -1.0f));
    yaw = -135.0f; 
    pitch = -30.0f;

    current_mode = 0;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        bullet_manager->update(deltaTime);
        bullet_manager->checkCollisions(current_scene->objects);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderID);
        
        glm::vec3 light_position(0.0f,10.0f,0.0f);

        GLuint lightPosLoc = glGetUniformLocation(shaderID, "lightPos");
        glUniform3f(lightPosLoc, light_position.x, light_position.y, light_position.z);
        
        GLuint objectColorLoc = glGetUniformLocation(shaderID, "objectColor");
        glUniform3f(objectColorLoc, 0.8f, 0.8f, 0.8f);


        specify_view();
        specify_projection();
        setup_light_uniforms(shaderID, light_position);

        GLuint loc = glGetUniformLocation(shaderID, "model");

        for (auto obj : current_scene->objects) {
            if (loc != -1) {
                glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(obj->transform));
            }
            if (obj->mesh){
                for (auto group : obj->mesh->groups) {
                    setup_default_material(shaderID);
                    if (group->material){
                        std::string directory = obj->obj_file.substr(0, obj->obj_file.find_last_of("/\\"));
                        setup_material_uniforms(shaderID, group->material, directory);
                    }
                    glBindVertexArray(group->VAO);
                    glDrawArrays(GL_TRIANGLES, 0, group->vert_count);
                }
            }
            if (obj->is_animated){
                obj->update(deltaTime);
            }
        }
        glBindVertexArray(0);

        bullet_manager->render(shaderID);

        if (current_mode == 0) {  
            glDisable(GL_DEPTH_TEST); 
            render_control_points();
            glEnable(GL_DEPTH_TEST);
        }

        current_scene->update();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    current_scene->cleanup();
    glfwTerminate();
    return 0;
}