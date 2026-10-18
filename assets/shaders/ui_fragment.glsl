uniform sampler2D g_texture;

in vec4 color;
in vec2 uv;

out vec4 outColor;

void main() {
    outColor = color * texture(g_texture, uv);
}
