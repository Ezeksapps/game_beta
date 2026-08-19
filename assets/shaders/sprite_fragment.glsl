uniform sampler2DArray g_texture;

in vec2 uv;
in flat float texArrayIndex;
in flat int instIdFS;
in vec3 inputPoint;

out vec4 color;

void main() {
    color = texture(g_texture, vec3(uv, texArrayIndex));
    //color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
}
