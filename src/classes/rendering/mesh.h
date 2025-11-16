#include <vector>
#include <algorithm>
#include "group.h"


class Mesh {
private:
    void process_vertex(const glm::ivec3& indices, 
    std::vector<glm::vec3>& vs,
    std::vector<glm::vec2>& ms,
    std::vector<glm::vec3>& ns
    );
public:
    std::vector<glm::vec3> verts;
    std::vector<glm::vec2> mappings;
    std::vector<glm::vec3> normals;
    std::vector<Group*> groups;
    void process_data();
};