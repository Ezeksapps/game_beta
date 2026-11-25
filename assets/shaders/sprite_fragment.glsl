uniform sampler2DArray g_texture;
// optional
uniform sampler2DArray g_overlay;

in DrawData {
    vec2 uv;
    flat float texArrayIndex;
    // optional, only used if using an overlay and that overlay is animated
    flat float overlayArrayIndex;
    flat float hasOverlay;
} FS_in;


out vec4 color;

void main() {

    vec4 baseTexel = texture(g_texture, vec3(FS_in.uv, FS_in.texArrayIndex));

    if (FS_in.hasOverlay == 0.0f) color = baseTexel;
    else {
        // https://stackoverflow.com/a/39780654
        // The second texture overlays the sprite's texture, therefore use the base sprite texture's alpha channel for blending
        vec4 overlayTexel = texture(g_overlay, vec3(FS_in.uv, FS_in.overlayArrayIndex));
        // (from docs.gl) mix performs a linear interpolation between x and y using a to weight between them
        color.rgb = mix(baseTexel.rgb, overlayTexel.rgb, baseTexel.a);
        color.a = baseTexel.a;
    }
}
