#pragma once
#include "ofMain.h"

#define STRINGIFY(A) #A

class Environment{
public:
    string gl3VertShader;
    string gl3FragShader;
    
    Environment(){
        gl3VertShader = "#version 150\n";
        gl3VertShader += STRINGIFY(uniform mat4 projectionMatrix;
                                   uniform mat4 modelViewMatrix;
                                   uniform mat4 modelViewProjectionMatrix;

                                   in vec4 position;
                                   in vec3 normal;

                                   out vec4 positionVarying;
                                   out vec3 normalVarying;
                                   
                                   out mat4 normalMatrix;
                                   
                                   void main(){
                                       normalMatrix = inverse(transpose(modelViewMatrix));
                                       normalVarying = normal;
                                       positionVarying = position;
                                       gl_Position = modelViewProjectionMatrix * position;
                                   });
        
        gl3FragShader = "#version 150\n";
        gl3FragShader += STRINGIFY(out vec4 fragColor;
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
                                   });
    }
};