#if !N_GRAPHICS_SHADER_H
#define N_GRAPHICS_SHADER_H 1

#if !N_GRAPHICS_CPU
#define NBool bool
#define U32 uint
#define I32 int
#define F32 float
#define F64 double

#define N_Vec2_B   bvec2
#define N_Vec2_I32 ivec2
#define N_Vec2_U32 uvec2
#define N_Vec2_F32 vec2
#define N_Vec2_F64 dvec2

#define N_Vec3_B   bvec3
#define N_Vec3_I32 ivec3
#define N_Vec3_U32 uvec3
#define N_Vec3_F32 vec3
#define N_Vec3_F64 dvec3

#define N_Vec4_B   bvec4
#define N_Vec4_I32 ivec4
#define N_Vec4_U32 uvec4
#define N_Vec4_F32 vec4
#define N_Vec4_F64 dvec4
#define N_ARGB_F32 vec4
#define N_ARGB_U8 uint

#define STRUCT(name, definition) name { definition }

#define n_buffer_size_x(name) name##_size_x
#define n_buffer_size_y(name) name##_size_y
#define n_buffer_size_z(name) name##_size_z
#define n_buffer_size_w(name) name##_size_w

#define BUFFER(name, bind_id) \
layout(std430, binding = (bind_id) * 2 + 1) buffer name##_size \
{ \
    int n_buffer_size_x(name); \
    int n_buffer_size_y(name); \
    int n_buffer_size_z(name); \
    int n_buffer_size_w(name); \
}; \
layout(std430, binding = (bind_id) * 2) buffer name

#else

typedef N_Vec4_I32 N_GraphicsBufferLength;

#endif

#endif // !N_GRAPHICS_SHADER_H
