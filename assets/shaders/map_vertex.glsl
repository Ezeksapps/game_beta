uniform Constants {
    mat4 g_projMatrix;
    mat4 g_viewMatrix;
};

layout(location = 0) in vec3 pos;
layout(location = 2) in vec2 in_uv; // NOTE: not handled in this stage, handled in FS

out vec3 debugPos;
out vec2 uv;

void main() {
    gl_Position = g_projMatrix * g_viewMatrix * vec4(pos, 1.0);
    debugPos = pos;

    // Colour works, vertices are somehow not bound or set properly and draw nothing
    // Bypass matrices, map the vertex directly to NDC
    // Map pos.xy from [-1, 1] to screen space [-2, 2] with aspect ratio
   //float aspect = 800.0 / 600.0; // Hardcode for now
    //gl_Position = vec4(pos.x * 0.5, pos.y * 0.5, 0.0, 1.0); // works, fix matrices
    uv = in_uv; // texture UVs are not handled in this shader, pass down to next stage (FS)
}
