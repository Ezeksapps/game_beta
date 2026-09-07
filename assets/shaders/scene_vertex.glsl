uniform Constants {
    mat4 g_projMatrix;
    mat4 g_viewMatrix;
};

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 in_uv; // NOTE: not handled in this stage, handled in FS

out vec2 uv;

void main() {
    gl_Position = g_projMatrix * g_viewMatrix * vec4(pos, 1.0);
    uv = in_uv; // texture UV are not handled in this shader, pass down to next stage (FS)
}
