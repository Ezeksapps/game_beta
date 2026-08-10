flat out int instanceID;

// TODO: This needs to be able to handle non-vulkan systems as well (OpenGL uses gl_InstanceID)
// https://stackoverflow.com/questions/35638512/instanced-glsl-shaders-in-vulkan

void main() {
    instanceID = gl_InstanceIndex;
}
