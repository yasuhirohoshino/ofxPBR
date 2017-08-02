// default
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

// depth camera's view projection matrix
uniform mat4 lightsViewProjectionMatrix;

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

out vec3 mv_normalVarying;
out vec4 mv_positionVarying;

void SendPBRVaryings(){
    if (renderDepthMap == true) {
		// depth map pass
		m_positionVarying = (inverse(viewMatrix) * modelViewMatrix) * position;
		gl_Position = lightsViewProjectionMatrix * m_positionVarying;
	} else {
		// render pass
		m_positionVarying = (inverse(viewMatrix) * modelViewMatrix) * position;
		mat4 normalMatrix = inverse(transpose(modelViewMatrix));
		mv_positionVarying = modelViewMatrix * position;
		mv_normalVarying = vec3(mat3(normalMatrix) * normal);
		texCoordVarying = texcoord;
		colorVarying = color;
		gl_Position = modelViewProjectionMatrix * position;
	}
}