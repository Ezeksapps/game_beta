layout(binding = 0, std140) uniform Constants {
  mat4 g_projMatrix;
  mat4 g_viewMatrix;
};

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// Inputs from vertex shader – these are arrays of size 1 (since it's a point)
in mat4 modelMatrix[];
in flat float texArrayIndx[];
in flat float maxU[];
in flat float maxV[];

out vec2 uv;
out flat float texArrayIndex;

void main() {

  // The data for this point is at index 0 because there's only one vertex
  mat4 model = modelMatrix[0];
  float maxUVal = maxU[0];
  float maxVVal = maxV[0];
  texArrayIndex = texArrayIndx[0];

  // World-space position from the model matrix
  vec3 pos = (model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  // pos = vec3(float(instID) * 2.0, 0.0, 0.0);

  // Camera axes
  vec3 cameraRight = vec3(g_viewMatrix[0][0], g_viewMatrix[1][0], g_viewMatrix[2][0]);
  vec3 cameraUp = vec3(g_viewMatrix[0][1], g_viewMatrix[1][1], g_viewMatrix[2][1]);

  mat4 viewProjMatrix = g_projMatrix * g_viewMatrix;
  float size = 0.5;

  // Generate billboard quad – use maxUVal and maxVVal, not arrays
  gl_Position = viewProjMatrix * vec4(pos - cameraRight * size - cameraUp * size, 1.0);
  uv = vec2(0.0, maxVVal);
  EmitVertex();

  gl_Position = viewProjMatrix * vec4(pos - cameraRight * size + cameraUp * size, 1.0);
  uv = vec2(0.0, 0.0);
  EmitVertex();

  gl_Position = viewProjMatrix * vec4(pos + cameraRight * size - cameraUp * size, 1.0);
  uv = vec2(maxUVal, maxVVal);
  EmitVertex();

  gl_Position = viewProjMatrix * vec4(pos + cameraRight * size + cameraUp * size, 1.0);
  uv = vec2(maxUVal, 0.0);
  EmitVertex();

  EndPrimitive();
}
