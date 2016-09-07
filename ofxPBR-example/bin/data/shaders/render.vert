#version 150

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;
uniform mat4 viewMatrix;

in vec4 position;
in vec3 normal;
in vec2 texcoord;

out vec4 positionVarying;
out vec3 normalVarying;
out vec3 v_normalVarying;
out vec2 texCoordVarying;

out mat4 normalMatrix;

void main(){
    texCoordVarying = texcoord;
    normalMatrix = inverse(transpose((modelViewMatrix)));
    normalVarying = normal;
    v_normalVarying = normalize(mat3(normalMatrix) * normalVarying);
    positionVarying = (inverse(viewMatrix) * modelViewMatrix) * position;
    gl_Position = modelViewProjectionMatrix * position;
}