layout(location = 0) in vec3 pos;
layout(location = 1) in mat4 modelMatrix_in;
layout(location = 2) in int  texArrayIndex_in;

out mat4 modelMatrix;
out int  texArrayIndex_gs;

void main() {
    gl_Position = vec4(pos, 1.0f); // NOTE: shader compiler will crash if you dont use the uniforms

    // model matrix and texture array index not handled here, pass down to next stage (GS)
    modelMatrix = modelMatrix_in;
    texArrayIndex_gs = texArrayIndex_in;
}
