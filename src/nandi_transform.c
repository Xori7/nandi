#include "nandi.h"

extern void n_transform_update_matrix(NTransform *transform) {
    NMatrix4x4 scale;
    glm_mat4_identity((vec4*)&scale);
    glm_scale((vec4*)&scale, (float*)&transform->scale);

    NMatrix4x4 rotation;
    glm_quat_mat4((float*)&transform->rotation, (vec4*)&rotation);

    NMatrix4x4 translation;
    glm_mat4_identity((vec4*)&translation);
    glm_translate((vec4*)&translation, (float*)&transform->position);

    NMatrix4x4 scaleRotation;
    glm_mat4_mul((vec4*)&scale, (vec4*)&rotation, (vec4*)&scaleRotation);
    glm_mat4_mul((vec4*)&scaleRotation, (vec4*)&translation, (vec4*)&transform->matrix);
}

extern void n_transform_set_position(NTransform *transform, NVec3f32 position) {
    transform->position = position;
    n_transform_update_matrix(transform);
}

extern void n_transform_set_rotation(NTransform *transform, NQuaternion rotation) {
    transform->rotation = rotation;
    n_transform_update_matrix(transform);
}

extern void n_transform_set_rotation_euler(NTransform *transform, NVec3f32 rotation) {
    NQuaternion qx;
    NQuaternion qy;
    NQuaternion qz;
    NQuaternion qs;
    glm_quat((float*)&qx, glm_rad(rotation.x), 1, 0, 0);
    glm_quat((float*)&qy, glm_rad(rotation.y), 0, 1, 0);
    glm_quat((float*)&qz, glm_rad(rotation.z), 0, 0, 1);
    glm_quat_mul((float*)&qy, (float*)&qx, (float*)&qs);
    glm_quat_mul((float*)&qs, (float*)&qz, (float*)&transform->rotation);
    printf("rotation: %3.2f %3.2f %3.2f, transformRotation: %3.2f, %3.2f, %3.2f, %3.2f\n", rotation.x, rotation.y, rotation.z, transform->rotation.x, transform->rotation.y, transform->rotation.z, transform->rotation.w);
    n_transform_update_matrix(transform);
}

extern void n_transform_set_scale(NTransform *transform, NVec3f32 scale) {
    transform->scale = scale;
    n_transform_update_matrix(transform);
}
