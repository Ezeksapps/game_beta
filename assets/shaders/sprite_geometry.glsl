uniform Constants {
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

layout(location = 0) in mat4 modelMatrix[];
layout(location = 1) in int  texArrayIndex_gs[];

out vec2 uv; // CHECK: is this actually needed? (only set to the last value 1.0, 0.0, then used in frag??)
out int  texArrayIndex;

void main() {
    /* Generate billboard & calculate matrix based on camera position */

    // TODO: will only use first index in UVs and matrices, even if multiple sprites exist

    vec2 base_uv = tex_uv[0];            // Get UV from the vertex

    vec3 pos = gl_in[0].gl_Position.xyz; // get position vector of primitive (centre point of billboard)

    vec3 cameraRight = vec3(g_viewMatrix[0][0], g_viewMatrix[1][0], g_viewMatrix[2][0]);
    vec3 cameraUp = vec3(g_viewMatrix[0][1], g_viewMatrix[1][1], g_viewMatrix[2][1]);

    mat4 viewProjMatrix = g_projMatrix * g_viewMatrix * modelMatrix[0]; // NOTE: must mul by model matrix for proper position

    float size = 0.5;

    gl_Position = viewProjMatrix * vec4(pos - cameraRight * size - cameraUp * size, 1.0);
    uv = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = viewProjMatrix * vec4(pos - cameraRight * size + cameraUp * size, 1.0);
    uv = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = viewProjMatrix * vec4(pos + cameraRight * size - cameraUp * size, 1.0);
    uv = vec2(1.0, 1.0);
    EmitVertex();

    gl_Position = viewProjMatrix * vec4(pos + cameraRight * size + cameraUp * size, 1.0);
    uv = vec2(1.0, 0.0);
    EmitVertex();

    EndPrimitive();

    texArrayIndex = texArrayIndex_gs;
}
