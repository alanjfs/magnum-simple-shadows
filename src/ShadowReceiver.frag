uniform float shadowBias;
uniform sampler2DShadow shadowmapTexture;
uniform highp vec3 lightDirection;

in mediump vec3 transformedNormal;
in highp vec3 shadowCoord;

out lowp vec4 color;

void main() {
    /* You might want to source this from a texture or a vertex color */
    vec3 albedo = vec3(0.5, 0.5, 0.5);

    /* You might want to source this from a uniform */
    vec3 ambient = vec3(0.5, 0.5, 0.5);

    mediump vec3 normalizedTransformedNormal = normalize(transformedNormal);

    float inverseShadow = 1.0;

    /* Is the normal of this face pointing towards the light? */
    lowp float intensity = dot(normalizedTransformedNormal, lightDirection);

    /* Pointing away from the light anyway, we know it's in the shade, don't
       bother shadow map lookup */
    if (intensity <= 0) {
        inverseShadow = 0.0f;
        intensity = 0.0f;

    } else {
        inverseShadow = texture(shadowmapTexture, vec3(
            shadowCoord.xy,
            shadowCoord.z - shadowBias)
        );
    }

    color.rgba = vec4((ambient + vec3(intensity * inverseShadow)) * albedo, 1.0);
}
