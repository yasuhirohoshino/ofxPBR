#version 150

out vec4 fragColor;
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 viewMatrix;

//uniform sampler2DArray depthMap;
uniform sampler2DArray depthMap;
uniform int index;
uniform int numLights;

in vec3 normalVarying;
in vec4 positionVarying;
in vec2 texCoordVarying;

uniform mat4 shadowMatrix[8];
uniform mat4 shadowMat;
vec4 v_shadowCoord[8];
uniform vec3 lightPos[8];
//uniform vec4 colors[8];

in vec3 v_normalVarying;

vec2 poissonDisk[8] = vec2[](
                             vec2(0.9265977740287781, 0.36850881576538086),
                             vec2(0.20805540680885315, 0.2816232442855835),
                             vec2(0.4912513196468353, 0.8650672435760498),
                             vec2(-0.3647611141204834, 0.7251957058906555),
                             vec2(-0.6926340460777283, 0.036871228367090225),
                             vec2(-0.0503573939204216, -0.4144868552684784),
                             vec2(-0.5594532489776611, -0.8005858063697815), 
                             vec2(0.6502251625061035, -0.2589254677295685)
                             );

void main (void) {
    vec3 color = vec3(0.0);
    vec4 v_positionVarying = viewMatrix * positionVarying;
    for(int i=0;i<8;i++){
        if(i < numLights){
            vec4 shadowCoord = shadowMatrix[i] * v_positionVarying;
            float visiblity = 1.0;
            vec3 projCoords = shadowCoord.xyz / shadowCoord.w;
            if(projCoords.x <= 0.0 || projCoords.y <= 0.0 ||
               projCoords.x >= 1.0 || projCoords.y >= 1.0 ||
               projCoords.z >= 1.0 || projCoords.z <= 0.0){
                visiblity = 1.0;
            }else{
                float currentDepth = projCoords.z;
                if(currentDepth - 0.0001 > texture(depthMap, vec3(projCoords.xy, i)).r){
                    visiblity -= 1.0 / 9.0;
                }
                for(int j=0;j<8;j++){
                    if(currentDepth - 0.0001 > texture(depthMap, vec3(projCoords.xy + poissonDisk[j] * (1.0 / (1024.0 * 2)), i)).r){
                        visiblity -= 1.0 / 9.0;
                    }
                }
            }
            float lambert = max(dot(normalize(vec3(vec4(lightPos[i], 1.0))), normalVarying), 0.0);
            color += 1.0 * lambert * visiblity;
        }else{
            break;
        }
    }
    fragColor = vec4(color / vec3(float(numLights)), 1.0);
//    float depth = texture(depthMap, vec3(texCoordVarying.xy, 0)).r;
//    fragColor = vec4(vec3(depth, 0.0, 0.0), 1.0);
}