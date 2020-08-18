uniform highp mat4 modelMatrix;
uniform highp mat4 transformationProjectionMatrix;
uniform highp mat4 shadowmapMatrix;

in highp vec4 position;
in mediump vec3 normal;

out mediump vec3 transformedNormal;

out highp vec3 shadowCoord;

void main() {
    transformedNormal = mat3(modelMatrix) * normal;

    vec4 worldPos4 = modelMatrix * position;
    shadowCoord = (shadowmapMatrix * worldPos4).xyz;
    gl_Position = transformationProjectionMatrix * position;
}
