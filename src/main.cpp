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

#include "classes/rendering/3D/scene.h"
#include "classes/rendering/3D/obj3dwriter.h"

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
        // Ambient
        vec3 ambient;
        if(useDiffuseTexture) {
            ambient = light.ambient * texture(material.diffuse, TexCoord).rgb;
        } else {
            ambient = light.ambient * material.ambient;
        }
        
        // Diffuse 
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(light.position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse;
        if(useDiffuseTexture) {
            diffuse = light.diffuse * diff * texture(material.diffuse, TexCoord).rgb;
        } else {
            diffuse = light.diffuse * diff * material.diffuseColor;
        }
        
        // Specular
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
int currentObjectIndex;

// 0 = track editor / 1 = 3D model viewer
int current_mode = 0;

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

void track_editor(double xpos, double ypos){

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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
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
        return 0;
    }
    
    return textureID;
}

void setup_material_uniforms(GLuint shaderProgram, const std::shared_ptr<Material>& material, const std::string& textureDirectory) {
    // Load textures if needed
    if (material->needs_texture_loading()) {
        if (!material->diffuseMap.empty() && material->diffuse_texture == 0) {
            material->diffuse_texture = load_texture_from_file(material->diffuseMap, textureDirectory);
            std::cout << "Loaded diffuse texture: " << material->diffuseMap << " ID: " << material->diffuse_texture << std::endl;
        }
        if (!material->specularMap.empty() && material->specular_texture == 0) {
            material->specular_texture = load_texture_from_file(material->specularMap, textureDirectory);
            std::cout << "Loaded specular texture: " << material->specularMap << " ID: " << material->specular_texture << std::endl;
        }
        if (!material->normalMap.empty() && material->normal_texture == 0) {
            material->normal_texture = load_texture_from_file(material->normalMap, textureDirectory);
            std::cout << "Loaded normal texture: " << material->normalMap << " ID: " << material->normal_texture << std::endl;
        }
    }
    
    // Set material uniforms - match the names in your shader
    glUniform1i(glGetUniformLocation(shaderProgram, "useDiffuseTexture"), material->has_diffuse_texture());
    glUniform1i(glGetUniformLocation(shaderProgram, "useSpecularTexture"), material->has_specular_texture());
    
    if (material->has_diffuse_texture()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material->diffuse_texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "material.diffuse"), 0);
        // Also set color fallbacks
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuseColor"), 
                   1.0f, 1.0f, 1.0f); // White when using texture
    } else {
        glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuseColor"), 
                   material->diffuse.r, material->diffuse.g, material->diffuse.b);
    }
    
    if (material->has_specular_texture()) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material->specular_texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "material.specular"), 1);
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specularColor"), 
                   1.0f, 1.0f, 1.0f); // White when using texture
    } else {
        glUniform3f(glGetUniformLocation(shaderProgram, "material.specularColor"), 
                   material->specular.r, material->specular.g, material->specular.b);
    }
    
    // Set ambient and shininess
    glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), 
               material->ambient.r, material->ambient.g, material->ambient.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), material->shininess);
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
}

void error_log(int cod, const char * description) {
    std::cout << "GLFW Error (" << cod << "): " << description << std::endl;
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
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetErrorCallback(error_log);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    shaderID = setup_shader();

    //STARTUP LOGIC
    //cameraPos = glm::vec3(0.0f, 10.0f, 0.0f); 
    //cameraFront = glm::vec3(0.0f, -50.0f, -1.0f);
    current_scene = std::make_unique<Scene>();
    for (auto obj : Obj3DWriter::file_reader())
    {
        current_scene->add_object(obj);
        Obj3DWriter::write(obj);
    }

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

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

            //object rotation for testing
            //obj->transform = glm::rotate(obj->transform, 0.01f, glm::vec3(1.0));
            
            for (auto group : obj->mesh->groups) {
                if (group->material){
                    std::string directory = obj->obj_file.substr(0, obj->obj_file.find_last_of("/\\"));
                    setup_material_uniforms(shaderID, group->material, directory);
                }
                glBindVertexArray(group->VAO);
                glDrawArrays(GL_TRIANGLES, 0, group->vert_count);
            }
        }
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    current_scene->cleanup();
    glfwTerminate();
    return 0;
}