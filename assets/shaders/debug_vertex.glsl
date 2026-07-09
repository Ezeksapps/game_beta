uniform Constants {
    mat4 g_projMatrix;
    mat4 g_viewMatrix;
};
layout(location = 0) in vec3 pos;

void main() {
    gl_Position = g_projMatrix * g_viewMatrix * vec4(pos, 1.0);
}
