#version 410

#define MAX_LIGHTS 8

#define MAX_VERTICES 24

layout(triangles) in;
layout(triangle_strip, max_vertices = MAX_VERTICES) out;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

uniform mat4 viewMat[MAX_LIGHTS];
uniform int numLights;

in vec4  geomPosition[3];
in vec3  geomNormal[3];
in vec2  geomTexcoord[3];
in vec4  geomColor[3];
in mat4 geomNormalMatrix[3];

out vec4 positionVarying;
out vec3 normalVarying;
out vec2 texCoordVarying;
out vec4 colorVarying;
out mat4 normalMatrix;

out float layer;

uniform bool renderForDepthMap;

void render(){
    for(int i=0;i<3;i++){
        texCoordVarying = geomTexcoord[i];
        normalMatrix = geomNormalMatrix[i];
        normalVarying = geomNormal[i];
        positionVarying = geomPosition[i];
        colorVarying = geomColor[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}

void shadow(){
    for(int i=0; i<MAX_LIGHTS; i++){
        if(i < numLights){
            gl_Layer = i;
            for(int j=0;j<3;j++){
                layer = float(i);
                positionVarying = gl_in[j].gl_Position;
                gl_Position = viewMat[i] * positionVarying;
                EmitVertex();
            }
            EndPrimitive();
        }else{
            break;
        }
    }
}

void main() {
	if(renderForDepthMap == true){
		shadow();
	}else{
		render();
	}
}