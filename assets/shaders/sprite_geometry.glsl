layout(binding = 0) uniform Constants {
  mat4 g_projMatrix;
  mat4 g_viewMatrix;
};

layout(binding = 1) uniform SpriteConstants {
  float g_maxWidth;
  float g_maxHeight;
  float g_pxToCoordRatio;
};

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// NOTE: All inputs to a geometry shader must be arrays, even if they have only one element
in mat4 modelMatrix[];
in flat float texArrayIndx[];
in flat float width[];
in flat float height[];

out vec2 uv;
out flat float texArrayIndex;

void main() {

  // The data for this point is at index 0 because there's only one vertex
  mat4 model = modelMatrix[0];
  float spriteWidth  = width[0];
  float spriteHeight = height[0];
  texArrayIndex = texArrayIndx[0];

  // World-space position from the model matrix
  vec3 pos = (model * vec4(0.0, 0.0, 0.0, 1.0)).xyz;

  // Camera axes
  vec3 cameraRight = vec3(g_viewMatrix[0][0], g_viewMatrix[1][0], g_viewMatrix[2][0]);
  vec3 cameraUp = vec3(g_viewMatrix[0][1], g_viewMatrix[1][1], g_viewMatrix[2][1]);

  mat4 viewProjMatrix = g_projMatrix * g_viewMatrix;

  float halfW = spriteWidth  * g_pxToCoordRatio * 0.5;
  float halfH = spriteHeight * g_pxToCoordRatio * 0.5;

  float maxUVal = spriteWidth  / g_maxWidth;
  float maxVVal = spriteHeight / g_maxHeight;

  gl_Position = viewProjMatrix * vec4(pos - cameraRight * halfW - cameraUp * halfH, 1.0);
  uv = vec2(0.0, maxVVal);
  EmitVertex();

  gl_Position = viewProjMatrix * vec4(pos - cameraRight * halfW + cameraUp * halfH, 1.0);
  uv = vec2(0.0, 0.0);
  EmitVertex();

  gl_Position = viewProjMatrix * vec4(pos + cameraRight * halfW - cameraUp * halfH, 1.0);
  uv = vec2(maxUVal, maxVVal);
  EmitVertex();

  gl_Position = viewProjMatrix * vec4(pos + cameraRight * halfW + cameraUp * halfH, 1.0);
  uv = vec2(maxUVal, 0.0);
  EmitVertex();

  EndPrimitive();
}
