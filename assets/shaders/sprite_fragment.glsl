uniform sampler2DArray g_texture;

in vec2 uv;
flat in float  texArrayIndex;

out vec4 color;

void main() {
    color = texture(g_texture, vec3(uv, texArrayIndex));
    //if (texArrayIndex == 0) color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
