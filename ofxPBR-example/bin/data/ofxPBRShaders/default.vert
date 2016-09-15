#version 410

//#define MAX_LIGHTS 8

//uniform mat4 projectionMatrix;
//uniform mat4 modelViewMatrix;
//uniform mat4 textureMatrix;
//uniform mat4 modelViewProjectionMatrix;
//uniform vec4 globalColor;
//uniform mat4 viewMatrix;
//
//uniform bool renderForDepthMap;
//
//in vec4  position;
//in vec3  normal;
//in vec2  texcoord;
//in vec4  color;
//
//out vec4  geomPosition;
//out vec3  geomNormal;
//out vec2  geomTexcoord;
//out vec4  geomColor;
//
//
//subroutine void renderType();
//subroutine uniform renderType renderModel;
//
//subroutine(renderType)
//void shadow(){
//    gl_Position = (inverse(viewMatrix) * modelViewMatrix) * position;
//}
//
//subroutine(renderType)
//void render(){
//    geomPosition = position;
//    geomNormal = normal;
//    geomTexcoord = texcoord;
//    geomColor = color;
//    gl_Position = modelViewProjectionMatrix * position;
//}
//
//void main() {
//    renderModel();
//}

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
        gl_Position = (inverse(viewMatrix) * modelViewMatrix) * position;
    }else{
        normalMatrix = inverse(transpose(modelViewMatrix));
        geomNormal = normal;
        geomPosition = position;
        geomTexcoord = texcoord;
        geomColor = color;
        gl_Position = modelViewProjectionMatrix * position;
    }
}