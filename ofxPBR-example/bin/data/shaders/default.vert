#version 330

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;
uniform mat4 viewMatrix;

in vec4  position;
in vec3  normal;
in vec2  texcoord;
in vec4  color;

void main() {
    gl_Position = (inverse(viewMatrix) * modelViewMatrix) * position;
}