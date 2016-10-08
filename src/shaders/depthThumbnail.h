#pragma once
#include "ofMain.h"

#define STRINGIFY(A) #A

class DepthThumbnail{
public:
    string gl3VertShader;
    string gl3FragShader;
    
    DepthThumbnail(){
        gl3VertShader = "#version 150\n";
        gl3VertShader += STRINGIFY(uniform mat4 projectionMatrix;
                                   uniform mat4 modelViewMatrix;
                                   uniform mat4 textureMatrix;
                                   uniform mat4 modelViewProjectionMatrix;
                                   uniform mat4 inverseProjectionMatrix;
                                   
                                   in vec4  position;
                                   in vec4  color;
                                   in vec3  normal;
                                   in vec2  texcoord;
                                   
                                   out vec2 texCoordVarying;
                                   
                                   void main() {
                                       texCoordVarying = texcoord;
                                       gl_Position = modelViewProjectionMatrix * position;
                                   });
        
        gl3FragShader = "#version 150\n";
        gl3FragShader += STRINGIFY(uniform sampler2DArray rawSampler;
                                   uniform int index;
                                   in vec2 texCoordVarying;
                                   
                                   out vec4 fragColor;
                                   
                                   void main() {
                                       fragColor = vec4(texture(rawSampler, vec3(texCoordVarying.x, 1.0 - texCoordVarying.y, index)).r, 0, 0, 1);
                                   });
    }
};