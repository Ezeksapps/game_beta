uniform sampler2DArray g_texture;

in layout(location = 0) vec4 svPos;
in layout(location = 1) vec4 color;
in layout(location = 2) vec2 uv;

out vec4 outColor;

void main() {
    outColor = color * texture(g_texture, vec3(uv, 0.0));
}
