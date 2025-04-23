#if !N_GRAPHICS_SHADER_H
#define N_GRAPHICS_SHADER_H 1

#if !N_CPU
#define I32 int
#define F32 float
#define N_Vec2_F32 vec2
#define N_Vec3_F32 vec3
#define N_Vec4_F32 vec4
#define N_ARGB_F32 vec4
#define STRUCT(name, definition) name { definition }

#define BUFFER(name, bind_id) \
layout(std430, binding = (bind_id) * 2 + 1) buffer name##_size \
{ \
    int name##_size_x;\
    int name##_size_y;\
    int name##_size_z;\
    int name##_size_w;\
}; \
layout(std430, binding = (bind_id) * 2) buffer name

#define N_GraphicsBufferLength(name)
#else

typedef N_Vec4_I32 N_GraphicsBufferLength;

#endif

#endif // !N_GRAPHICS_SHADER_H
