uniform sampler2DArray g_texture;

in vec2 uv;
in flat float texArrayIndex;

out vec4 color;

void main() {
    color = texture(g_texture, vec3(uv, texArrayIndex));
    //if (color == vec4(0.0f, 0.0f, 0.0f, 0.0f)) color = vec4(0.0f, 0.2f, 1.0f, 1.0f);
}
