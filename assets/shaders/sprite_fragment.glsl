uniform sampler2DArray g_texture;

in vec2 uv;
flat in float texArrayIndex;

out vec4 color;

void main() {
    color = texture(g_texture, vec3(uv, texArrayIndex));//texArrayIndex));
    // DEBUG: Used to test if texArrayIndex is being properly set //
    //if (texArrayIndex == 0.0f) color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    //else if (texArrayIndex > 0.0f) color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}
