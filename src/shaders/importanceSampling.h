#pragma once
#include "ofMain.h"

#define STRINGIFY(A) #A

// based on https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf

class ImportanceSampling{
public:
    string gl3VertShader;
    string gl3FragShader;
    
    ImportanceSampling(){
        gl3VertShader = "#version 150\n";
        gl3VertShader += STRINGIFY(uniform mat4 orientationMatrix;
                                   uniform mat4 projectionMatrix;
                                   uniform mat4 modelViewMatrix;
                                   uniform mat4 textureMatrix;
                                   uniform mat4 modelViewProjectionMatrix;
                                   
                                   in vec4 position;
                                   in vec4 color;
                                   in vec3 normal;
                                   in vec2 texcoord;
                                   
                                   out vec3 normalVarying;
                                   
                                   void main() {
                                       normalVarying = normal;
                                       gl_Position = modelViewProjectionMatrix * position;
                                   });
        
        gl3FragShader = "#version 150\n";
        gl3FragShader += STRINGIFY(const float PI = 3.14159265358979;
                                   
                                   uniform samplerCube envMap;
                                   uniform float Roughness;
								   uniform float faceResolution;
                                   
                                   in vec3 normalVarying;
                                   
                                   out vec4 fragColor;
                                   
                                   float radicalInverse_VdC(uint bits) {
                                       bits = (bits << 16u) | (bits >> 16u);
                                       bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
                                       bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
                                       bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
                                       bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
                                       return float(bits) * 2.3283064365386963e-10;
                                   }
                                   
                                   vec2 Hammersley(uint i, uint N) {
                                       return vec2(
                                                   float(i) / float(N),
                                                   radicalInverse_VdC(i)
                                                   );
                                   }
                                   
                                   vec3 ImportanceSampleGGX( vec2 Xi, float Roughness, vec3 N ) {
                                       float a = Roughness * Roughness;
                                       
                                       float Phi = 2 * PI * Xi.x;
                                       float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
                                       float SinTheta = sqrt( 1 - CosTheta * CosTheta );
                                       
                                       vec3 H;
                                       H.x = SinTheta * cos( Phi );
                                       H.y = SinTheta * sin( Phi );
                                       H.z = CosTheta;
                                       
                                       vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
                                       vec3 TangentX = normalize( cross( UpVector, N ) );
                                       vec3 TangentY = cross( N, TangentX );
                                       return TangentX * H.x + TangentY * H.y + N * H.z;
                                   }
                                   
                                   float G_Smith(float Roughness, float NoV, float NoL) {
                                       float a = Roughness * Roughness;
                                       float k = a / 2.0;
                                       
                                       float G1l = NoL / (NoL * (1.0 - k) + k);
                                       float G1v = NoV / (NoV * (1.0 - k) + k);
                                       
                                       float Glvn = G1l * G1v;
                                       return Glvn;
                                   }

								   float DistributionGGX(vec3 N, vec3 H, float roughness)
								   {
									   float a = roughness * roughness;
									   float a2 = a * a;
									   float NdotH = max(dot(N, H), 0.0);
									   float NdotH2 = NdotH * NdotH;

									   float nom = a2;
									   float denom = (NdotH2 * (a2 - 1.0) + 1.0);
									   denom = PI * denom * denom;

									   return nom / denom;
								   }

                                   vec3 prefilterEnvMap(float Roughness, vec3 R){
                                       vec3 N = R;
                                       vec3 V = R;
                                       vec3 PrefilteredColor = vec3(0.0);
                                       
                                       const int NumSamples = 1024;
                                       float totalsample = 0.0;
                                       
                                       for(int i=0; i<NumSamples; i++) {
                                           vec2 Xi = Hammersley(uint(i), uint(NumSamples));
                                           vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
                                           vec3 L = 2.0 * dot(V, H) * H - V;
                                           
                                           float NoL = clamp(dot(N, L), 0, 1);
                                           
                                           if (NoL > 0.0) {
											   // https://learnopengl.com/PBR/IBL/Specular-IBL
											   // sample from the environment's mip level based on roughness/pdf
											   float D = DistributionGGX(N, H, Roughness);
											   float NdotH = max(dot(N, H), 0.0);
											   float HdotV = max(dot(H, V), 0.0);
											   float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

											   float resolution = faceResolution; // resolution of source cubemap (per face)
											   float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
											   float saSample = 1.0 / (float(NumSamples) * pdf + 0.0001);

											   float mipLevel = Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

											   vec3 SampleColor = textureLod(envMap, L, mipLevel).rgb * NoL;

                                               PrefilteredColor += SampleColor;
											   totalsample += NoL;
                                           }
                                       }
                                       return PrefilteredColor / totalsample;
                                   }
                                   
                                   void main (void) {
                                       fragColor = vec4(prefilterEnvMap(Roughness, normalVarying),1.0);
                                   });
    }
};