uniform sampler2DArray g_texture;

in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(g_texture, vec3(uv, 0.0));
}
