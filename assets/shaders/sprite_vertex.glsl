in layout(location = 0) mat4 mdlMatrix;
in layout(location = 4) float texArrayIdx;
in layout(location = 5) float mxU;
in layout(location = 6) float mxV;


out mat4 modelMatrix;
out flat float texArrayIndx;
out flat float maxU;
out flat float maxV;

flat out int instanceID;

// vertex shader not really needed here, but left in case needed in future

// TODO: This needs to be able to handle non-vulkan systems as well (OpenGL uses gl_InstanceID)
// https://stackoverflow.com/questions/35638512/instanced-glsl-shaders-in-vulkan

void main() {
    modelMatrix = mdlMatrix;
    texArrayIndx = texArrayIdx;
    maxU = mxU;
    maxV = mxV;
    instanceID = gl_InstanceIndex;
}
