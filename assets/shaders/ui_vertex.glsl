layout(binding = 0) uniform Constants {
    mat4 g_projMatrix; // Not the main projection matrix, this is an ortho proj specifically for UI
    mat4 g_viewMatrix; // same view matrix from frame constants
};

in layout(location = 0) vec2 pos;
in layout(location = 1) vec2 vs_uv;
in layout(location = 2) vec4 vs_color;

out vec4 color;
out vec2 uv;

void main() {
    gl_Position = g_projMatrix * g_viewMatrix * vec4(pos.xy, 0.0f, 1.0f);
    color = vs_color;
    uv = vs_uv;
}

