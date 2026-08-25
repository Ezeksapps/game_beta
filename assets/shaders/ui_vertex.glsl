layout(binding = 0) uniform Constants {
    mat4 g_projMatrix;
    mat4 g_viewMatrix; // UNUSED FOR UI (position of UI elems does not depend on camera position)
};

in layout(location = 0) float pos;
in layout(location = 1) float vs_uv;
in layout(location = 2) float vs_col;

out float svPos;
out float uv;
out float col;

void main() {
    svPos = vec4(pos.xy, 0.0f, 1.0f), g_projMatrix);
    col = vs_col;
    uv  = vs_uv;
}

// TODO: check layout, location vals may be off
