// default
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

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

out mat4 normalMatrix;

void SendPBRVaryings(){
    normalMatrix = inverse(transpose(modelViewMatrix));
	normalVarying = normal;
    positionVarying = position;
    texCoordVarying = texcoord;
    colorVarying = color;
    gl_Position = modelViewProjectionMatrix * position;
}