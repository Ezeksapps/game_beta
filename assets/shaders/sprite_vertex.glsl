in layout(location = 0) mat4 mdlMatrix;
in layout(location = 4) float texArrayIdx;
in layout(location = 5) float ovrlArrayIdx;
in layout(location = 6) float usingOverlay;
in layout(location = 7) float wdth;
in layout(location = 8) float hght;

out mat4 modelMatrix;
out flat float texArrayIndx;
out flat float ovrlArrayIndx;
out flat float usingOverl;
out flat float width;
out flat float height;

void main() {
    modelMatrix = mdlMatrix;
    texArrayIndx = texArrayIdx;
    ovrlArrayIndx = ovrlArrayIdx;
    usingOverl = usingOverlay;
    width  = wdth;
    height = hght;
}
