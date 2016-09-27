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
out vec4 positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;
out vec4 colorVarying;
out vec4 mPositionVarying;
out mat4 normalMatrix;

void main() {
    if (renderForDepthMap == true) {
        mPositionVarying = (inverse(viewMatrix) * modelViewMatrix) * position;
        gl_Position = viewMat * mPositionVarying;
    } else {
        mPositionVarying = (inverse(viewMatrix) * modelViewMatrix) * position;
        normalMatrix = inverse(transpose(modelViewMatrix));
        normalVarying = normal;
        positionVarying = position;
        texCoordVarying = texcoord;
        colorVarying = color;
        gl_Position = modelViewProjectionMatrix * position;
    }
}