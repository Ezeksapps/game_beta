layout(location = 0) in mat4 modelMatrix;
layout(location = 4) in float texArrayIndex;
layout(location = 5) in float overlayArrayIndex;
layout(location = 6) in float hasOverlay;
layout(location = 7) in float width;
layout(location = 8) in float height;

out VertexData {
    mat4 modelMatrix;
    flat float texArrayIndex;
    flat float overlayArrayIndex;
    flat float hasOverlay;
    flat float width;
    flat float height;
} VS_out;

void main() {
    VS_out.modelMatrix = modelMatrix;
    VS_out.texArrayIndex = texArrayIndex;
    VS_out.overlayArrayIndex = overlayArrayIndex;
    VS_out.hasOverlay = hasOverlay;
    VS_out.width = width;
    VS_out.height = height;
}

