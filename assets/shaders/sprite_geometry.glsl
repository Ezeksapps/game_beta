layout(binding = 0, std140) uniform Constants {
    mat4 g_projMatrix;
    mat4 g_viewMatrix;
};



/* Refs for implementation:
 * https://gamedev.stackexchange.com/questions/113147/rotate-billboard-towards-camera
 * https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
 * https://ogldev.org/www/tutorial27/tutorial27.html
 */

// for sprites, geometry shader calculates and creates the billboard quad
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in mat4 modelMatrix[];
in flat float texArrayIndx[];
in flat float maxU[];
in flat float maxV[];
in flat int instanceID[];

out vec2 uv;
out flat float texArrayIndex;
out flat int instIdFS;
//out vec3 inputPoint;

void main() {

    int instID = 0;

    instIdFS = instID;
    //instID = 0;

    /* Generate billboard & calculate matrix based on camera position */
    mat4 model = modelMatrix[instID];

    // World‑space position
    vec3 pos = (model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    //inputPoint = pos;

    // Camera axes
    vec3 cameraRight = vec3(g_viewMatrix[0][0], g_viewMatrix[1][0], g_viewMatrix[2][0]);
    vec3 cameraUp = vec3(g_viewMatrix[0][1], g_viewMatrix[1][1], g_viewMatrix[2][1]);

    mat4 viewProjMatrix = g_projMatrix * g_viewMatrix;

    float size = 0.5;

    texArrayIndex = texArrayIndx[instID];

    gl_Position = viewProjMatrix * vec4(pos - cameraRight * size - cameraUp * size, 1.0);
    uv = vec2(0.0, maxV[instID]);
    EmitVertex();

      //  texArrayIndex = texIdx;
    gl_Position = viewProjMatrix * vec4(pos - cameraRight * size + cameraUp * size, 1.0);
    uv = vec2(0.0, 0.0);
    EmitVertex();

       // texArrayIndex = texIdx;
    gl_Position = viewProjMatrix * vec4(pos + cameraRight * size - cameraUp * size, 1.0);
    uv = vec2(maxU[instID], maxV[instID]);
    EmitVertex();

       // texArrayIndex = texIdx;
    gl_Position = viewProjMatrix * vec4(pos + cameraRight * size + cameraUp * size, 1.0);
    uv = vec2(maxU[instID], 0.0);
    EmitVertex();

    EndPrimitive();
}
