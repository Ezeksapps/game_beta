uniform sampler2D g_texture;

in vec4 color;
in vec2 uv;

out vec4 outColor;

void main() {
    outColor = color * texture(g_texture, uv);
    //float a = texture(g_texture, uv).a;
    //outColor = vec4(a, a, a, 1.0);
}
