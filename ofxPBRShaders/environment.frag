#version 150

out vec4 fragColor;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform samplerCube envMap;
uniform int numMips;
uniform float envLevel;

uniform float cubeMapExposure;
uniform float cubeMapRotation;

in vec3 normalVarying;
in vec4 positionVarying;

vec3 PrefilterEnvMap(float Roughness, vec3 R) {
    float rot = cubeMapRotation;
    mat3 rotationMatrix = mat3(cos(rot), 0, sin(rot), 0, 1, 0, -sin(rot), 0, cos(rot));
    vec3 rotatedVector = rotationMatrix * R;
    vec4 color = mix(textureLod( envMap, rotatedVector, int(Roughness * numMips) ), textureLod( envMap, rotatedVector, min(int(Roughness * numMips) + 1, numMips)), fract(Roughness * numMips));
    return color.rgb * cubeMapExposure;
}

void main (void) {
    fragColor = vec4(PrefilterEnvMap(envLevel, normalVarying), 1.0);
}