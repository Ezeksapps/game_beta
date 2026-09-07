uniform sampler2D g_texture;

in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(g_texture, uv);
}
