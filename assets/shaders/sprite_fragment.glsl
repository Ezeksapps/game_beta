uniform sampler2DArray g_texture;

in vec2 uv;
in flat float texArrayIndex;
in flat int instIdFS;
in vec3 inputPoint;

out vec4 color;

void main() {
    color = texture(g_texture, vec3(uv, texArrayIndex));
}
