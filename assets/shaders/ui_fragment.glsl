uniform sampler2DArray g_texture;

in layout(location = 0) float svPos;
in layout(location = 1) float uv;
in layout(location = 2) float col;

out vec4 color;

void main() {
    //return vs_output.col * texture0.Sample(texture0_sampler, vs_output.uv);
    color = texture(g_texture, vec3(uv, 0.0)); // add col consideration
}
