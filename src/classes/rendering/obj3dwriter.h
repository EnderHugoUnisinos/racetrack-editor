#include <vector>
#include <string>

#include "obj3d.h"

class Obj3DWriter{
public:
    static void write(Obj3D* obj);
    static Mesh* load_from_file(const std::string& filename);
    static std::vector<Obj3D*> file_reader();
};