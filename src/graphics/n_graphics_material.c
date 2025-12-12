#include "nandi/n_graphics.h"

struct N_Material {
    U32 id;
    const N_Shader *shader;
    N_MaterialProperties properties;
};

static N_Material materials[MAX_MATERIALS_COUNT];
static U32 material_count = 0;

extern const N_Material* n_graphics_material_create(const N_Shader *shader, N_MaterialProperties properties) {
    N_Material *material = &materials[material_count];
    material->id = material_count++;
    material->shader = shader;
    material->properties = properties;
    return material;
}

extern void n_graphics_material_destroy(N_Material *material) {
    *material = (N_Material){0};
}

extern const N_Shader* n_graphics_material_get_shader(const N_Material *material) {
    return material->shader;
}

extern void n_graphics_material_set_properties(N_Material *material, N_MaterialProperties properties) {
    material->properties = properties;
}

extern N_MaterialProperties* n_graphics_material_get_properties(const N_Material *material) {
    return (N_MaterialProperties*)&material->properties;
}

