uniform sampler2DArray g_texture;

in vec3 debugPos;
in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(g_texture, vec3(uv, 0.0));
   // outColor = vec4(debugPos * 0.2 + 0.5, 1.0);
    //outColor = texture(g_texture, vec3(uv, 0.0)); // Only works when all vertices have textures
//    outColor = vec4(1.0, 0.0, 0.0, 1.0); // displays nothing (should be overriding first line)
}
