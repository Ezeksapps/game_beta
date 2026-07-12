// TODO: Should handle per-instance transform
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv; // NOTE: not handled in this stage, handled in FS
layout(location = 2) in mat4 modelMatrix_in;

out vec2 tex_uv;
out mat4 modelMatrix;

void main() {
    gl_Position = vec4(pos, 1.0f); // NOTE: shader compiler will crash if you dont use the uniforms

    // texture UVs are not handled in this shader, pass down to next stage (GS)
    tex_uv = uv;
    modelMatrix = modelMatrix_in;
}
