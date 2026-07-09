uniform Constants {
    mat4 g_projMatrix;
    mat4 g_viewMatrix;
};

// TODO: Should handle per-instance transform
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv; // NOTE: not handled in this stage, handled in FS

out vec2 tex_uv;

void main() {
    gl_Position = g_projMatrix * g_viewMatrix * vec4(pos, 1.0); // NOTE: shader compiler will crash if you dont use the uniforms

    // texture UVs are not handled in this shader, pass down to next stage (GS)
    tex_uv = uv;
}
