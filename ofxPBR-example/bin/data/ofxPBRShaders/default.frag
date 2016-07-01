#version 330

#define MAX_LIGHTS 8

#pragma include "pbrFrag.glsl"

out vec4 fragColor;

vec3 baseColor = vec3(0.0);
float roughness = 0.0;
float metallic = 0.0;
vec3 normal = vec3(0.0);
vec3 reflectDir = vec3(0.0);
float occlusion = 1.0;
vec3 color = vec3(0.0);
vec3 emissionColor = vec3(0.0);

void main (void) {
    if(renderForDepthMap == true){
        fragColor = vec4(1.0);
    } else {
        SetParams();
        // Set Gamma from cubemap
        SetGamma();
        // Set BaseColor
        baseColor = GetBaseColor(colorVarying, texCoordVarying);
        // Set Roughness
        roughness = GetRoughness(texCoordVarying);
        // Set Metallic
        metallic = GetMetallic(texCoordVarying);
        // Set Normal and Reflection
        normal = vec3(0.0);
        reflectDir = vec3(0.0);
        CalcNormal(normal, reflectDir, texCoordVarying);
        // Set Occlusion
        occlusion = GetOcclusion(texCoordVarying);
        // Set Color
        color = CalcColor(baseColor, roughness, metallic, normal, reflectDir, occlusion);
        // Set Emission color
        emissionColor = GetEmissionColor(texCoordVarying);
        // Apply Emission or not
        color = DetectEmission(color, emissionColor);
        fragColor = vec4(color, 1.0);
    }
}