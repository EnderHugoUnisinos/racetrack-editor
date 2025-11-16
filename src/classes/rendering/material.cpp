#include "material.h"
#include <stb_image.h>

void Material::cleanup() {
    if (diffuse_texture) {
        glDeleteTextures(1, &diffuse_texture);
        diffuse_texture = 0;
    }
    if (specular_texture) {
        glDeleteTextures(1, &specular_texture);
        specular_texture = 0;
    }
    if (normal_texture) {
        glDeleteTextures(1, &normal_texture);
        normal_texture = 0;
    }
}
