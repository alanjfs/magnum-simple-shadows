uniform float shadowBias;
uniform sampler2DShadow shadowmapTexture;
uniform highp vec3 lightDirection;

in mediump vec3 transformedNormal;
in highp vec3 shadowCoord;

out lowp vec4 color;

void main() {
    /* You might want to source this from a texture or a vertex color */
    vec3 albedo = vec3(0.5,0.5,0.5);

    /* You might want to source this from a uniform */
    vec3 ambient = vec3(0.5,0.5,0.5);

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
        int shadowLevel = 0;
        bool inRange = false;

        /* Starting with highest resolution shadow map, find one we're
           in range of */
        for(; shadowLevel < 1; ++shadowLevel) {
            vec3 shadowCoord_ = shadowCoord;
            inRange = shadowCoord_.x >= 0 &&
                      shadowCoord_.y >= 0 &&
                      shadowCoord_.x <  1 &&
                      shadowCoord_.y <  1 &&
                      shadowCoord_.z >= 0 &&
                      shadowCoord_.z <  1;
            if(inRange) {
                inverseShadow = texture(shadowmapTexture, vec3(
                    shadowCoord_.xy,
                    shadowCoord_.z - shadowBias)
                );
                break;
            }
        }

        if(!inRange) {
            // If your shadow maps don't cover your entire view, you might want to remove this
            albedo *= vec3(1,0,1); //Something has gone wrong - didn't find a shadow map
        }
    }

    color.rgb = ((ambient + vec3(intensity*inverseShadow))*albedo);
    color.a = 1.0;
}
