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

layout(location = 0) in vec2 tex_uv[];

out vec2 uv; // may no longer be needed in FS?

void main() {
    /* Generate billboard & calculate matrix based on camera position */

    vec2 base_uv = tex_uv[0];            // Get UV from the vertex

    vec3 pos = gl_in[0].gl_Position.xyz; // get position vector of primitive (centre point of billboard)

    vec3 cameraRight = vec3(g_viewMatrix[0][0], g_viewMatrix[1][0], g_viewMatrix[2][0]);
    vec3 cameraUp = vec3(g_viewMatrix[0][1], g_viewMatrix[1][1], g_viewMatrix[2][1]);

    float size = 0.5;

    // Calculate view-projection matrix
    mat4 viewProjMatrix = g_projMatrix * g_viewMatrix;

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
}
