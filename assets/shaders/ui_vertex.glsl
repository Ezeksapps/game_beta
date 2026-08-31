layout(binding = 0) uniform Constants {
    mat4 g_projMatrix; // Not the main projection matrix, this is an ortho proj specifically for UI
};

in layout(location = 0) vec2 pos;
in layout(location = 1) vec2 vs_uv;
in layout(location = 2) vec4 vs_col;

out vec4 svPos;
out vec4 col;
out vec2 uv;


void main() {
    svPos = vec4(pos.xy, 0.0f, 1.0f) * g_projMatrix;
    col = vs_col;
    uv  = vs_uv;
}

// TODO: check layout, location vals may be off
