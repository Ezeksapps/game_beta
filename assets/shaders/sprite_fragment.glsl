uniform sampler2DArray g_texture;
// optional
uniform sampler2DArray g_overlay;

in bool usingOverlay;
in vec2 uv;
in flat float texArrayIndex;
// optional, only used if using an overlay and that overlay is animated
in flat float overlaySliceIndex;

out vec4 color;

void main() {

    if (!usingOverlay) color = texture(g_texture, vec3(uv, texArrayIndex));
    else {
        // https://stackoverflow.com/a/39780654
        // The second texture overlays the sprite's texture, therefore use the base sprite texture's alpha channel for blending
        vec3 baseTexel = texture(g_texture, uv).rgb;
        vec3 overlayTexel = texture(g_overlay, uv).rgb
        // (from docs.gl) mix performs a linear interpolation between x and y using a to weight between them
        color.rgb = mix(baseTexel, overlayTexel, g_texture.a);
    }
}
