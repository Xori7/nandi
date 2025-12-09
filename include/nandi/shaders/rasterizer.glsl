BUFFER(frame_buffer, 0)
    uint imageData[];
};

BUFFER(vertex_buffer, 1)
    Vert vertices[];
};

BUFFER(global_buffer, 2)
    N_ShaderGlobal global;
};

BUFFER(index_buffer, 3)
    int indices[];
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

void compute(int x, int y) {
    int width = n_buffer_size(frame_buffer).x;
    int height = n_buffer_size(frame_buffer).y;
}

Frag vert(Vert vert);
vec4 frag(vec3 baricentric, Frag v0, Frag v1, Frag v2);

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = ivec2(n_buffer_size(frame_buffer).xy);

    if (any(greaterThanEqual(pixel, size))) return;

    uint pixelIndex = uint(pixel.y * size.x + pixel.x);
    vec4 finalColor = vec4(0.0, 0.0, 0.0, 1.0);

    uint triangleStart = 0;
    uint triangleCount = 100;

    // Loop over ALL triangles that could cover this pixel
    for (uint i = triangleStart; i < triangleStart + triangleCount; i++)
    {
        uint idx = i * 3;
        uint i0 = indices[idx];
        uint i1 = indices[idx+1];
        uint i2 = indices[idx+2];

        Frag f0 = vert(vertices[i0]);
        Frag f1 = vert(vertices[i1]);
        Frag f2 = vert(vertices[i2]);

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
            imageData[pixelIndex] = packUnorm4x8(color.bgra);
            return;
        }
    }
}
