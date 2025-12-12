layout(std430, binding = 0) buffer global_buff {
    ivec4 global_size;
    N_ShaderGlobal global;
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_Vertices {
    ivec4 size;
    Vert data[];
};

layout(buffer_reference, scalar, buffer_reference_align = 8) readonly buffer N_Indices {
    ivec4 size;
    uint data[];
};

layout (local_size_x = WORKGROUP_SIZE, local_size_y = WORKGROUP_SIZE, local_size_z = 1) in;

N_ShaderGlobal get_global() {
    return global;
}

float linear_to_srgb(float linear) {
    if (linear <= 0.0031308)
        return 12.92 * linear;
    return 1.055 * pow(linear, 1.0 / 2.4) - 0.055;
}

vec3 PointInTriangle (vec2 p, vec2 p0, vec2 p1, vec2 p2)
{
    float A = 0.5 * (-p1.y * p2.x + p0.y * (-p1.x + p2.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y);
    float s = 1.0 / (2*A)*(p0.y*p2.x - p0.x*p2.y + (p2.y - p0.y)*p.x + (p0.x - p2.x)*p.y);
    float t = 1.0 / (2*A)*(p0.x*p1.y - p0.y*p1.x + (p0.y - p1.y)*p.x + (p1.x - p0.x)*p.y);
    float r = 1 - s - t;
    
    return vec3(r, s, t);
}

Frag vert(Vert vert);
vec4 frag(vec3 baricentric, Frag v0, Frag v1, Frag v2);

void main() {
    N_Texture render_texture = N_Texture(get_global().render_texture);
    N_Indices indices = N_Indices(get_global().index_buffer);
    N_Vertices vertices = N_Vertices(get_global().vertex_buffer);

    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = ivec2(render_texture.size.xy);

    if (any(greaterThanEqual(pixel, size))) return;
    
    /*
    if (gl_WorkGroupID.x % 2 == 0 || gl_WorkGroupID.y % 2 == 0) {
        return;
    }
    */

    uint pixelIndex = uint(pixel.y * size.x + pixel.x);
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);

    uint triangleStart = 0;
    uint triangleCount = 100;


    // Loop over ALL triangles that could cover this pixel
    for (uint i = triangleStart; i < triangleStart + triangleCount; i++)
    {
        uint idx = i * 3;
        uint i0 = indices.data[idx];
        uint i1 = indices.data[idx+1];
        uint i2 = indices.data[idx+2];

        Frag f0 = vert(vertices.data[i0]);
        Frag f1 = vert(vertices.data[i1]);
        Frag f2 = vert(vertices.data[i2]);

        // CORRECT FOR SCREEN-SPACE (0..1) → pixel coordinates
        vec2 p0 = f0.position.xy * vec2(size);
        vec2 p1 = f1.position.xy * vec2(size);
        vec2 p2 = f2.position.xy * vec2(size);

        // Fast bounding box reject
        vec2 bbMin = floor(min(min(p0, p1), p2));
        vec2 bbMax = ceil(max(max(p0, p1), p2));
        if (any(lessThan(vec2(pixel), bbMin)) || 
            any(greaterThanEqual(vec2(pixel), bbMax)))
            continue;

        vec3 bary = PointInTriangle(vec2(pixel), p0, p1, p2);
        if (all(greaterThanEqual(bary, vec3(0.0))))
        {
            vec4 color = frag(bary, f0, f1, f2);
            render_texture.pixels[pixelIndex] = color.rgba;
            return;
        }
    }
}
