layout(location = 0) in mat4 modelMatrix_in;
layout(location = 4) in float  texArrayIndex_in;

out mat4 modelMatrix;
flat out float  texArrayIndex_gs;

void main() {
    // no vertex buffer exists because it isn't necessary, centre billboard pos = world-space pos = model matrix * vec4(0.0f, 0.0f, 0.0f, 1.0f)
    gl_Position = modelMatrix_in * vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // model matrix and texture array index not handled here, pass down to next stage (GS)
    modelMatrix = modelMatrix_in;
    texArrayIndex_gs = texArrayIndex_in;
}
