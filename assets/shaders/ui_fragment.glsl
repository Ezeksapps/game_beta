uniform sampler2DArray g_texture;

in layout(location = 0) vec4 svPos;
in layout(location = 1) vec4 col;
in layout(location = 2) vec2 uv;

out vec4 color;

void main() {
    color = col * texture(g_texture, vec3(uv, 0.0));
}
