#pragma once
#include "ofMain.h"

// Blur shader based on: http://callumhay.blogspot.ca/2010/09/gaussian-blur-shader-glsl.html

#define STRINGIFY(A) #A

class Blur{
public:
    string gl3VertShader;
    string gl3FragShader;
    
    Blur(){
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
        gl3FragShader += STRINGIFY(uniform float sigma;
                                   uniform sampler2D blurSampler;
                                   uniform vec2 resolution;
                                   uniform int horizontal;
                                   
                                   const float pi = 3.14159265;
                                   const int numBlurPixelsPerSide = 3;
                                   
                                   in vec2 texCoordVarying;
                                   
                                   out vec4 fragColor;
                                   
                                   void main() {
                                       
                                       vec2 blurMultiplyVec = vec2(0.0);
                                       if(horizontal == 1){
                                           blurMultiplyVec = vec2(1.0, 0.0) / resolution;
                                       }else{
                                           blurMultiplyVec = vec2(0.0, 1.0) / resolution;
                                       }
                                       
                                       vec3 incrementalGaussian;
                                       incrementalGaussian.x = 1.0 / (sqrt(2.0 * pi) * sigma);
                                       incrementalGaussian.y = exp(-0.5 / (sigma * sigma));
                                       incrementalGaussian.z = incrementalGaussian.y * incrementalGaussian.y;
                                       
                                       vec4 avgValue = vec4(0.0, 0.0, 0.0, 0.0);
                                       float coefficientSum = 0.0;
                                       
                                       avgValue += texture(blurSampler, texCoordVarying) * incrementalGaussian.x;
                                       coefficientSum += incrementalGaussian.x;
                                       incrementalGaussian.xy *= incrementalGaussian.yz;
                                       
                                       for (float i = 1.0; i <= numBlurPixelsPerSide; i++) {
                                           avgValue += texture(blurSampler, texCoordVarying - i * blurMultiplyVec) * incrementalGaussian.x;
                                           avgValue += texture(blurSampler, texCoordVarying + i * blurMultiplyVec) * incrementalGaussian.x;
                                           coefficientSum += 2.0 * incrementalGaussian.x;
                                           incrementalGaussian.xy *= incrementalGaussian.yz;
                                       }
                                       
                                       avgValue /= coefficientSum;
                                       
                                       fragColor = avgValue;
                                   });
    }
};