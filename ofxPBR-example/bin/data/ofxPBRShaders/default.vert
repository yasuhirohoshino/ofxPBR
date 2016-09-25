#version 410

const int MAX_LIGHTS = 8;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;
uniform mat4 viewMatrix;

uniform mat4 viewMat;
uniform bool renderForDepthMap;

// passThrough
// in
in vec4 position;
in vec3 normal;
in vec2 texcoord;
in vec4 color;
// out
out vec4  geomPosition;
out vec3  geomNormal;
out vec2  geomTexcoord;
out vec4  geomColor;

out mat4 normalMatrix;

void main(){
    if(renderForDepthMap == true){
        gl_Position = viewMat * (inverse(viewMatrix) * modelViewMatrix) * position;
    }else{
        normalMatrix = inverse(transpose(modelViewMatrix));
        geomNormal = normal;
        geomPosition = position;
        geomTexcoord = texcoord;
        geomColor = color;
        gl_Position = modelViewProjectionMatrix * position;
    }
}