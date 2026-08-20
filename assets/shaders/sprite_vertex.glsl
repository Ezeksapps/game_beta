in layout(location = 0) mat4 mdlMatrix;
in layout(location = 4) float texArrayIdx;
in layout(location = 5) float mxU;
in layout(location = 6) float mxV;


out mat4 modelMatrix;
out flat float texArrayIndx;
out flat float maxU;
out flat float maxV;

void main() {
    modelMatrix = mdlMatrix;
    texArrayIndx = texArrayIdx;
    maxU = mxU;
    maxV = mxV;
}
