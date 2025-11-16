#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

class Face {
public:
    std::vector<int> verts;
    std::vector<int> textures;
    std::vector<int> normals;
};