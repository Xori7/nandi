#if !N_MESH_H
#define N_MESH_H 1

#include "n_math.h"
#include "n_graphics.h"

typedef struct N_Mesh N_Mesh;

extern N_Mesh*      n_mesh_create(void);
extern void         n_mesh_destroy(const N_Mesh *mesh);
extern void         n_mesh_name_set(const N_Mesh *mesh, const char *name);
extern char*        n_mesh_name_get(const N_Mesh *mesh);

extern U32          n_mesh_vertices_count_get(const N_Mesh *mesh);
extern void         n_mesh_vertices_count_set(const N_Mesh *mesh, U32 count);

extern N_Vec2_F32*  n_mesh_vertices_map(const N_Mesh *mesh);
extern void         n_mesh_vertices_unmap(const N_Mesh *mesh);

extern N_Vec2_F32*  n_mesh_uvs_map(const N_Mesh *mesh, U32 channel);
extern void         n_mesh_uvs_unmap(const N_Mesh *mesh, U32 channel);

extern void         n_mesh_indices_count_set(const N_Mesh *mesh, U32 count);
extern void         n_mesh_indices_count_get(const N_Mesh *mesh, U32 count);

extern U32*         n_mesh_indices_map(const N_Mesh *mesh);
extern void         n_mesh_indices_unmap(const N_Mesh *mesh);

extern void         n_mesh_render(const N_Mesh *mesh, N_Shader *shader, N_Matrix4x4 matrix);
extern void         n_mesh_render_instanced(const N_Mesh *mesh, N_Shader *shader, U32 count, const N_Matrix4x4 *matrices);

#endif
