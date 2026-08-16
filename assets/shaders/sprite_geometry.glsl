layout(binding = 0) uniform Constants {
    mat4 g_projMatrix;
    mat4 g_viewMatrix;
};

struct InstanceData {
    mat4 modelMatrix;
    float texArrayIndex;
    float maxU;
    float maxV;
};

layout(binding = 1) uniform SpriteConstants {
    InstanceData g_instData[32];
};

/* Refs for implementation:
 * https://gamedev.stackexchange.com/questions/113147/rotate-billboard-towards-camera
 * https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
 * https://ogldev.org/www/tutorial27/tutorial27.html
 */

// for sprites, geometry shader calculates and creates the billboard quad
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

flat in int instanceID[];

out vec2 uv;
out flat float texArrayIndex;

void main() {

    int instID = instanceID[0];

    float maxU = g_instData[instID].maxU;
    float maxV = g_instData[instID].maxV;

    /* Generate billboard & calculate matrix based on camera position */
    texArrayIndex = g_instData[instID].texArrayIndex;

    mat4 model = g_instData[instID].modelMatrix;

    // World‑space position
    vec3 pos = (g_instData[instID].modelMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

    // Camera axes
    vec3 cameraRight = vec3(g_viewMatrix[0][0], g_viewMatrix[1][0], g_viewMatrix[2][0]);
    vec3 cameraUp = vec3(g_viewMatrix[0][1], g_viewMatrix[1][1], g_viewMatrix[2][1]);

    mat4 viewProjMatrix = g_projMatrix * g_viewMatrix;

    float size = 0.5;

    gl_Position = viewProjMatrix * vec4(pos - cameraRight * size - cameraUp * size, 1.0);
    uv = vec2(0.0, maxV);
    EmitVertex();

    gl_Position = viewProjMatrix * vec4(pos - cameraRight * size + cameraUp * size, 1.0);
    uv = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = viewProjMatrix * vec4(pos + cameraRight * size - cameraUp * size, 1.0);
    uv = vec2(maxU, maxV);
    EmitVertex();

    gl_Position = viewProjMatrix * vec4(pos + cameraRight * size + cameraUp * size, 1.0);
    uv = vec2(maxU, 0.0);
    EmitVertex();

    EndPrimitive();
}
