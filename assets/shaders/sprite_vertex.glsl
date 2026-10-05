in layout(location = 0) mat4 mdlMatrix;
in layout(location = 4) float texArrayIdx;
in layout(location = 5) float wdth;
in layout(location = 6) float hght;


out mat4 modelMatrix;
out flat float texArrayIndx;
out flat float width;
out flat float height;

void main() {
    modelMatrix = mdlMatrix;
    texArrayIndx = texArrayIdx;
    width  = wdth;
    height = hght;
}
