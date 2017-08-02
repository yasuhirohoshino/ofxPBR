#version 410

const int MAX_LIGHTS = 8;
const float PI = 3.14159265358979;
const float TwoPI = 6.28318530718;

const int MODE_PBR = 0;
const int MODE_SPOTSHADOW = 1;
const int MODE_OMNISHADOW = 2;
const int MODE_CASCADESHADOW = 3;
const int MODE_DIRECTIONALSHADOW = 4;

const int LIGHTTYPE_DIRECTIONAL = 0;
const int LIGHTTYPE_SPOT = 1;
const int LIGHTTYPE_POINT = 2;
const int LIGHTTYPE_SKY = 3;

const int SHADOWTYPE_NONE = 0;
const int SHADOWTYPE_HARD = 1;
const int SHADOWTYPE_SOFT = 2;

uniform int renderMode;

struct Light {
    bool isEnabled;
    vec3 position;
    vec3 vPosition;
    vec4 color;
    vec3 direction;
    int type;
    float farClip;
    float intensity;
    float spotLightFactor;
    float spotLightCutoff;
    float spotLightDistance;
    float pointLightRadius;
    int shadowType;
    int shadowIndex;
    int omniShadowIndex;
	int cascadeShadowIndex;
	int directionalShadowIndex;
    float shadowStrength;
    float bias;
};

// in
in vec4 colorVarying;
in vec2 texCoordVarying;
in vec4 m_positionVarying;
in vec3 mv_normalVarying;
in vec4 mv_positionVarying;

// Uniforms

// default uniforms
uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 textureMatrix;
uniform mat4 modelViewProjectionMatrix;
uniform vec4 globalColor;

// Lights
uniform Light lights[MAX_LIGHTS];
uniform int numLights;

// cameras
uniform float cameraNear;
uniform float cameraFar;

// Shadows
uniform sampler2DArrayShadow shadowMap;
uniform mat4 shadowMatrix[MAX_LIGHTS];
uniform samplerCubeArray omniShadowMap;
uniform sampler2DArrayShadow cascadeShadowMap;
uniform int numCascade;
uniform float cascadeClips[5];
uniform mat4 cascadeShadowMatrix[MAX_LIGHTS * 4];

uniform sampler2DArrayShadow directionalShadowMap;


// IBL
uniform samplerCube envMap;
uniform int numMips;
uniform int isHDR;
uniform float cubeMapExposure;
uniform float cubeMapRotation;

// Reflection
uniform mat4 viewTranspose;

// Material parameters
uniform vec4 baseColorUniform;
uniform float roughnessUniform;
uniform float metallicUniform;
uniform float normalValUniform;
uniform vec2 textureRepeatTimes;
uniform vec2 detailTextureRepeatTimes;

// Material textures
uniform int enableBaseColorMap;
uniform sampler2D baseColorMap;
uniform int enableRoughnessMap;
uniform sampler2D roughnessMap;
uniform int enableMetallicMap;
uniform sampler2D metallicMap;
uniform int enableNormalMap;
uniform sampler2D normalMap;
uniform int enableOcclusionMap;
uniform sampler2D occlusionMap;
uniform int enableEmissionMap;
uniform sampler2D emissionMap;
uniform int enableDetailBaseColorMap;
uniform sampler2D detailBaseColorMap;
uniform int enableDetailNormalMap;
uniform sampler2D detailNormalMap;
uniform int enableGlobalColor;

vec3 mv_normal;
float gamma = 1.0;
vec4 v_shadowCoord[MAX_LIGHTS];
vec4 v_cascadeShadowCoord[MAX_LIGHTS * 4];
vec4 v_directionalShadowCoord[MAX_LIGHTS];

void SetParams() {
	mv_normal = normalize(mv_normalVarying);
	for (int i = 0; i < MAX_LIGHTS; i++) {
		if (i <= numLights) {
			if(lights[i].type == LIGHTTYPE_SPOT){
				int index = lights[i].shadowIndex;
				v_shadowCoord[index] = shadowMatrix[index] * m_positionVarying;
			} 
			else if(lights[i].type == LIGHTTYPE_DIRECTIONAL || lights[i].type == LIGHTTYPE_SKY){
				//int index = lights[i].cascadeShadowIndex;
				//for(int j = 0; j < numCascade; j++){
				//	v_cascadeShadowCoord[index * numCascade + j] = cascadeShadowMatrix[index * numCascade + j] * m_positionVarying;
				//}

				int index = lights[i].directionalShadowIndex;
				v_directionalShadowCoord[index] = shadowMatrix[index] * m_positionVarying;
			}
		}
		else {
			break;
		}
	}
}

// IBL
vec3 PrefilterEnvMap(float Roughness, vec3 R) {
	float rot = cubeMapRotation;
	mat3 rotationMatrix = mat3(cos(rot), 0, sin(rot), 0, 1, 0, -sin(rot), 0, cos(rot));
	vec3 rotatedVector = rotationMatrix * R;
	vec4 color = mix(textureLod(envMap, rotatedVector, int(Roughness * numMips)), textureLod(envMap, rotatedVector, min(int(Roughness * numMips) + 1, numMips)), fract(Roughness * numMips));
	return  color.rgb * cubeMapExposure;
}

vec3 EnvBRDFApprox(vec3 SpecularColor, float Roughness, float NoV) {
	vec4 c0 = vec4(-1, -0.0275, -0.572, 0.022);
	vec4 c1 = vec4(1, 0.0425, 1.04, -0.04);
	vec4 r = Roughness * c0 + c1;
	float a004 = min(r.x * r.x, exp2(-9.28 * NoV)) * r.x + r.y;
	vec2 AB = vec2(-1.04, 1.04) * a004 + r.zw;
	return SpecularColor * AB.x + AB.y;
}

vec3 ApproximateSpecularIBL(vec3 SpecularColor, float Roughness, vec3 N, vec3 V, vec3 ReflectDir) {
	float NoV = dot(N, V);
	vec3 PrefilteredColor = PrefilterEnvMap(Roughness, ReflectDir);
	return SpecularColor * PrefilteredColor * EnvBRDFApprox(SpecularColor, Roughness, NoV);
}

// Fresnel
vec3 Fresnel(vec3 N, vec3 V, float Roughness, vec3 ReflectDir, float f0) {
	float base = 1.0 - clamp(dot(N, V), 0.0, 0.99);
	float exponential = pow(base, 5.0);
	float fresnel = f0 + (1.0 - f0) * exponential;
	vec3 reflectColor = PrefilterEnvMap(Roughness, ReflectDir);
	return reflectColor * fresnel;
}

// Utils
vec3 BlendSoftLight(vec3 Base, vec3 Blend) {
	return mix(
		sqrt(Base) * (2.0 * Blend - 1.0) + 2.0 * Base * (1.0 - Blend),
		2.0 * Base * Blend + Base * Base * (1.0 - 2.0 * Blend),
		step(Base, vec3(0.5))
	);
}

float Luma(vec3 Color) {
	return dot(Color, vec3(0.2126, 0.7152, 0.0722));
}

// Lighting

// https://github.com/stackgl/glsl-specular-cook-torrance/blob/master/index.glsl
float BeckmannDistribution(float x, float roughness) {
	float NdotH = max(x, 0.0001);
	float cos2Alpha = NdotH * NdotH;
	float tan2Alpha = (cos2Alpha - 1.0) / cos2Alpha;
	float roughness2 = max(roughness * roughness, 0.0001);
	float denom = 3.141592653589793 * roughness2 * cos2Alpha * cos2Alpha;
	return exp(tan2Alpha / roughness2) / denom;
}

// https://github.com/stackgl/glsl-specular-cook-torrance
float CookTorranceSpecular(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness) {
	float VdotN = max(dot(viewDirection, surfaceNormal), 0.0);
	float LdotN = max(dot(lightDirection, surfaceNormal), 0.0);

	//Half angle vector
	vec3 H = normalize(lightDirection + viewDirection);

	//Geometric term
	float NdotH = max(dot(surfaceNormal, H), 0.0);
	float VdotH = max(dot(viewDirection, H), 0.000001);
	float LdotH = max(dot(lightDirection, H), 0.000001);
	float G1 = (2.0 * NdotH * VdotN) / VdotH;
	float G2 = (2.0 * NdotH * LdotN) / LdotH;
	float G = min(1.0, min(G1, G2));

	//Distribution term
	float D = BeckmannDistribution(NdotH, roughness);

	//Fresnel term
	float F = pow(1.0 - VdotN, 0.02);

	//Multiply terms and done
	return  G * F * D / max(3.14159265 * VdotN, 0.000001);
}

// https://github.com/stackgl/glsl-diffenable-oren-nayar
float OrenNayarDiffuse(vec3 lightDirection, vec3 viewDirection, vec3 surfaceNormal, float roughness) {
	float LdotV = dot(lightDirection, viewDirection);
	float NdotL = dot(lightDirection, surfaceNormal);
	float NdotV = dot(surfaceNormal, viewDirection);

	float s = LdotV - NdotL * NdotV;
	float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

	float sigma2 = roughness * roughness;
	float A = 1.0 + sigma2 * (1.0 / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
	float B = 0.45 * sigma2 / (sigma2 + 0.09);

	return max(0.0, NdotL) * (A + B * s / t) / PI;
}

float Falloff(float dist, float lightRadius) {
	float att = clamp(1.0 - dist * dist / (lightRadius * lightRadius), 0.0, 1.0);
	att *= att;
	return att;
}

void CalcPointLight(vec3 normal, vec3 color, int index, float roughness, out vec3 deffuse, out vec3 specular) {
	vec3 s = normalize(lights[index].vPosition - mv_positionVarying.xyz);
	float dif = OrenNayarDiffuse(s, -normalize(mv_positionVarying.xyz), normal, roughness);
	float falloff = Falloff(length(lights[index].vPosition - mv_positionVarying.xyz), lights[index].pointLightRadius);
	float spec = CookTorranceSpecular(s, -normalize(mv_positionVarying.xyz), normal, roughness);
	deffuse = lights[index].intensity * (color.rgb * lights[index].color.rgb * dif * falloff);
	specular = lights[index].intensity * (color.rgb * lights[index].color.rgb * spec * falloff);
}

void CalcSpotLight(vec3 normal, vec3 color, int index, float roughness, out vec3 deffuse, out vec3 specular) {
	vec3 s = normalize(lights[index].vPosition - mv_positionVarying.xyz);
	float angle = acos(dot(-s, lights[index].direction));
	float cutoff1 = radians(clamp(lights[index].spotLightCutoff - max(lights[index].spotLightFactor, 0.01), 0.0, 89.9));
	float cutoff2 = radians(clamp(lights[index].spotLightCutoff, 0.0, 90.0));
	if (angle < cutoff2) {
		float dif = OrenNayarDiffuse(s, -normalize(mv_positionVarying.xyz), normal, roughness);
		float falloff = Falloff(length(lights[index].vPosition - mv_positionVarying.xyz), lights[index].spotLightDistance);
		float spec = CookTorranceSpecular(s, -normalize(mv_positionVarying.xyz), normal, roughness);
		deffuse = lights[index].intensity * (color.rgb * lights[index].color.rgb * dif * falloff) * smoothstep(cutoff2, cutoff1, angle);
		specular = lights[index].intensity * (color.rgb * lights[index].color.rgb * spec * falloff) * smoothstep(cutoff2, cutoff1, angle);
	}
	else {
		deffuse = vec3(0.0);
		specular = vec3(0.0);
	}
}

void CalcDirectionalLight(vec3 normal, vec3 color, int index, float roughness, out vec3 deffuse, out vec3 specular) {
	float dif = OrenNayarDiffuse(-lights[index].direction, -normalize(mv_positionVarying.xyz), normal, roughness);
	float spec = CookTorranceSpecular(-lights[index].direction, -normalize(mv_positionVarying.xyz), normal, roughness);
	deffuse = lights[index].intensity * (color.rgb * lights[index].color.rgb * dif);
	specular = lights[index].intensity * (color.rgb * lights[index].color.rgb * spec);
}

float CalcShadow(int index) {
	float visiblity = 1.0;
	int shadowIndex = lights[index].shadowIndex;
	vec3 projCoords = v_shadowCoord[shadowIndex].xyz / v_shadowCoord[shadowIndex].w;
	float currentDepth = projCoords.z;
	if (projCoords.x >= 1.0 || projCoords.x <= 0.0 ||
		projCoords.y >= 1.0 || projCoords.y <= 0.0 ||
		projCoords.z >= 1.0 || projCoords.z <= 0.0) {
		visiblity = 1.0;
	}else{
		if(lights[index].shadowType == SHADOWTYPE_SOFT){
			float offset = 1.0;
			visiblity = texture(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias));
			visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(-offset, -offset));
			visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(-offset, offset));
			visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(offset, offset));
			visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(offset, -offset));
			if(visiblity < 5.0){
				visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(0, offset));
				visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(offset, 0));
				visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(0, -offset));
				visiblity += textureOffset(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(-offset, 0));
				visiblity /= 9.0;
			}else{
				visiblity /= 5.0;
			}
		}else{
			visiblity = texture(shadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias));
		}
	}
	visiblity = 1.0 - (1.0 - visiblity) * lights[index].shadowStrength;
	return visiblity;
}

float CalcOmniShadow(int index, vec3 fragPos)
{
	float shadow = 0.0;
	float farPlane = lights[index].farClip;
	vec3 fragToLight = fragPos - lights[index].position;
	float closestDepth = texture(omniShadowMap, vec4(fragToLight, lights[index].omniShadowIndex)).r;
	if (closestDepth < 1.0) {
		float currentDepth = length(fragToLight);
		closestDepth *= (farPlane + farPlane * 0.001 + (farPlane * 0.1 * (currentDepth / farPlane)));
		shadow = currentDepth < closestDepth ? 1.0 : 0.0;
	}
	return shadow;
}

float CalcCascadeShadow(int index){
	float depth = -mv_positionVarying.z;
	float visiblity = 0.0;
	int shadowIndex = lights[index].cascadeShadowIndex;
	for(int i = 0; i < numCascade; i++){
		if(depth > cascadeClips[i] && depth <= cascadeClips[i + 1]){
			vec3 projCoords = v_cascadeShadowCoord[shadowIndex * numCascade + i].xyz / v_cascadeShadowCoord[shadowIndex * numCascade + i].w;
			float currentDepth = projCoords.z;

			if (projCoords.x >= 1.0 || projCoords.x <= 0.0 ||
				projCoords.y >= 1.0 || projCoords.y <= 0.0 ||
				projCoords.z >= 1.0 || projCoords.z <= 0.0) {
				visiblity = 1.0;
			}
			else {
				if(lights[index].shadowType == SHADOWTYPE_SOFT){
					float offset = 1.0;
					visiblity = texture(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias));
					visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(-offset, -offset));
					visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(-offset, offset));
					visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(offset, offset));
					visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(offset, -offset));
					if(visiblity < 5.0){
						visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(0, offset));
						visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(offset, 0));
						visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(0, -offset));
						visiblity += textureOffset(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias), ivec2(-offset, 0));
						visiblity /= 9.0;
					}else{
						visiblity /= 5.0;
					}
				}else{
					visiblity = texture(cascadeShadowMap, vec4(projCoords.xy, shadowIndex * numCascade + i, projCoords.z - lights[index].bias));
				}
			}
		}
	}
	visiblity = 1.0 - (1.0 - visiblity) * lights[index].shadowStrength;
	return visiblity;
}

float CalcDirectionalShadow(int index) {
	float visiblity = 1.0;
	int shadowIndex = lights[index].directionalShadowIndex;
	vec3 projCoords = v_directionalShadowCoord[shadowIndex].xyz / v_directionalShadowCoord[shadowIndex].w;
	float currentDepth = projCoords.z;
	if (projCoords.x >= 1.0 || projCoords.x <= 0.0 ||
		projCoords.y >= 1.0 || projCoords.y <= 0.0 ||
		projCoords.z >= 1.0 || projCoords.z <= 0.0) {
		visiblity = 1.0;
	}else{
		if(lights[index].shadowType == SHADOWTYPE_SOFT){
			float offset = 1.0;
			visiblity = texture(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias));
			visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(-offset, -offset));
			visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(-offset, offset));
			visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(offset, offset));
			visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(offset, -offset));
			if(visiblity < 5.0){
				visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(0, offset));
				visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(offset, 0));
				visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(0, -offset));
				visiblity += textureOffset(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias), ivec2(-offset, 0));
				visiblity /= 9.0;
			}else{
				visiblity /= 5.0;
			}
		}else{
			visiblity = texture(directionalShadowMap, vec4(projCoords.xy, shadowIndex, projCoords.z - lights[index].bias));
		}
	}
	visiblity = 1.0 - (1.0 - visiblity) * lights[index].shadowStrength;
	return visiblity;
}

void LightWithShadow(vec3 normal, vec3 color, float roughness, int index, out vec3 deffuse, out vec3 specular) {
	vec3 lightDeffuse = vec3(0.0);
	vec3 lightSpecular = vec3(0.0);
	if (lights[index].type == LIGHTTYPE_DIRECTIONAL || lights[index].type == LIGHTTYPE_SKY) {
		CalcDirectionalLight(normal, color, index, roughness, lightDeffuse, lightSpecular);
	}
	else if (lights[index].type == LIGHTTYPE_SPOT) {
		CalcSpotLight(normal, color, index, roughness, lightDeffuse, lightSpecular);
	}
	else if (lights[index].type == LIGHTTYPE_POINT) {
		CalcPointLight(normal, color, index, roughness, lightDeffuse, lightSpecular);
	}

	float shadow = 1.0;
	if (lights[index].shadowType != SHADOWTYPE_NONE) {
		if(lights[index].type == LIGHTTYPE_DIRECTIONAL || lights[index].type == LIGHTTYPE_SKY){
			shadow = CalcDirectionalShadow(index);
		}else if(lights[index].type == LIGHTTYPE_POINT){
			shadow = CalcOmniShadow(index, m_positionVarying.xyz);
		}else{
			shadow = CalcShadow(index);
		}
	}

	deffuse = vec3(lightDeffuse * clamp(shadow, 0.0, 1.0));
	specular = clamp(lightSpecular, 0.0, 1.0) * clamp(shadow, 0.0, 1.0);
}

void SetGamma() {
	if (isHDR == 1) {
		gamma = 2.2;
	}
	else {
		gamma = 1.0;
	}
}

vec3 GetBaseColor(vec4 colorVarying, vec2 texCoordVarying) {
	vec3 baseColorFromParam = vec3(1.0);
	if (enableGlobalColor == 1) {
		baseColorFromParam = colorVarying.rgb;
	}
	else {
		baseColorFromParam = baseColorUniform.rgb;
	}
	if (enableBaseColorMap == 1) {
		vec3 baseColorFromTex = texture(baseColorMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).rgb;
		if (enableDetailBaseColorMap == 1) {
			vec3 detailBaseColor = texture(detailBaseColorMap, mod(texCoordVarying * detailTextureRepeatTimes, 1.0)).rgb;
			return pow(BlendSoftLight(baseColorFromTex, detailBaseColor) * baseColorFromParam.rgb, vec3(gamma));
		}
		else {
			return pow(baseColorFromTex * baseColorFromParam.rgb, vec3(gamma));
		}
	}
	else {
		return pow(baseColorFromParam.rgb, vec3(gamma));
	}
}

float GetRoughness(vec2 texCoordVarying) {
	if (enableRoughnessMap == 1) {
		return texture(roughnessMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).r;
	}
	else {
		return roughnessUniform;
	}
}

float GetMetallic(vec2 texCoordVarying) {
	if (enableMetallicMap == 1) {
		return texture(metallicMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).r;
	}
	else {
		return metallicUniform;
	}
}

// normal
// http://www.geeks3d.com/20130122/normal-mapping-without-precomputed-tangent-space-vectors/
// http://www.thetenthplanet.de/archives/1180
mat3 CotangentFrame(vec3 N, vec3 p, vec2 uv) {
	// get edge vectors of the pixel triangle
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
	vec2 duv1 = dFdx(uv);
	vec2 duv2 = dFdy(uv);

	// solve the linear system
	vec3 dp2perp = cross(dp2, N);
	vec3 dp1perp = cross(N, dp1);
	vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

	// construct a scale-invariant frame 
	float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
	return mat3(T * invmax, B * invmax, N);
}

vec3 PerturbNormal(vec3 normalMap, vec3 N, vec3 V, vec2 texcoord) {
	// assume N, the interpolated vertex normal and 
	// V, the view vector (vertex to eye)
	vec3 map = normalMap * 255. / 127. - 128. / 127.;
	mat3 TBN = CotangentFrame(N, -V, texcoord);
	return normalize(TBN * map);
}

void CalcNormal(out vec3 normal, out vec3 reflectDir, vec2 texCoordVarying) {
	if (enableNormalMap == 1) {
		vec3 normalMapVec = texture(normalMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).xyz;
		if (enableDetailNormalMap == 1) {
			vec3 detailNormalMapVec = texture(detailNormalMap, mod(texCoordVarying * detailTextureRepeatTimes, 1.0)).rgb;
			normalMapVec = BlendSoftLight(normalMapVec, detailNormalMapVec);
		}
		normal = mix(mv_normal, PerturbNormal(normalMapVec, mv_normal, mv_positionVarying.xyz, texCoordVarying), vec3(normalValUniform));
		vec3 relfect0 = reflect(normalize(mv_positionVarying.xyz), normal);
		reflectDir = vec3(viewTranspose * vec4(relfect0, 0.0)) * vec3(1, 1, -1);
	}
	else {
		normal = mv_normal;
		vec3 relfect0 = reflect(normalize(mv_positionVarying.xyz), mv_normal);
		reflectDir = vec3(viewTranspose * vec4(relfect0, 0.0)) * vec3(1, 1, -1);
	}
}

float GetOcclusion(vec2 texCoordVarying) {
	if (enableOcclusionMap == 1) {
		return pow(texture(occlusionMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).r, gamma);
	}
	else {
		return 1;
	}
}

vec3 CalcColor(vec3 baseColor, float roughnessVal, float metallicVal, vec3 normal, vec3 reflectDir, float occlusion) {
	vec3 viewDir = normalize(-mv_positionVarying.xyz);
	vec3 diffuseColor = baseColor - baseColor * metallicVal;
	vec3 specularColor = mix(vec3(0.01), baseColor, metallicVal);
	vec3 diffuse = PrefilterEnvMap(int(numMips * 2 / 3), normal) * diffuseColor;
	vec3 specular = ApproximateSpecularIBL(specularColor, roughnessVal, normal, viewDir, reflectDir);
	vec3 fresnel = Fresnel(normal, normalize(-mv_positionVarying.xyz), roughnessVal, reflectDir, 0.02) * specularColor * metallicVal;
	for (int i = 0; i < numLights; i++) {
		if (lights[i].isEnabled == true) {
			vec3 lightDeffuse;
			vec3 lightSpecular;
			LightWithShadow(normal, diffuseColor, roughnessVal, i, lightDeffuse, lightSpecular);
			diffuse += lightDeffuse;
			specular += lightSpecular;
		}
	}
	return vec3(diffuse + specular + fresnel) * occlusion;
}

vec3 GetEmissionColor(vec2 texCoordVarying) {
	if (enableEmissionMap == 1) {
		vec3 emissionColor = pow(texture(emissionMap, mod(texCoordVarying * textureRepeatTimes, 1.0)).rgb, vec3(gamma));
		if (Luma(emissionColor) > 0.0) {
			return emissionColor;
		}
		else {
			return vec3(0.0);
		}
	}
	else {
		return vec3(0.0);
	}
}

vec3 DetectEmission(vec3 color, vec3 emissionColor) {
	if (Luma(emissionColor) > 0.0) {
		return emissionColor;
	}
	else {
		return color;
	}
}

out vec4 fragColor;

vec3 baseColor = vec3(0.0);
float roughness = 0.0;
float metallic = 0.0;
vec3 normal = vec3(0.0);
vec3 reflectDir = vec3(0.0);
float occlusion = 1.0;
vec3 color = vec3(0.0);
vec3 emissionColor = vec3(0.0);

uniform vec3 lightPos;
uniform float farPlane;

void main(void) {
	if(renderMode == MODE_PBR){
		SetParams();
		// Set Gamma from cubemap
		SetGamma();
		// Set BaseColor
		baseColor = GetBaseColor(colorVarying, texCoordVarying);
		// Set Roughness
		roughness = GetRoughness(texCoordVarying);
		// Set Metallic
		metallic = GetMetallic(texCoordVarying);
		// Set Normal and Reflection
		CalcNormal(normal, reflectDir, texCoordVarying);
		// Set Occlusion
		occlusion = GetOcclusion(texCoordVarying);
		// Set Color
		color = CalcColor(baseColor, roughness, metallic, normal, reflectDir, occlusion);
		// Set Emission color
		emissionColor = GetEmissionColor(texCoordVarying);
		// Apply Emission or not
		color = DetectEmission(color, emissionColor);
		fragColor = vec4(color, 1.0);
		gl_FragDepth = gl_FragCoord.z;
	}
	else if(renderMode == MODE_SPOTSHADOW || renderMode == MODE_CASCADESHADOW || renderMode == MODE_DIRECTIONALSHADOW){
		fragColor = vec4(1.0);
		gl_FragDepth = gl_FragCoord.z;
		fragColor = vec4(gl_FragCoord.z, 0.0, 0.0, 1.0);
	}
	else if(renderMode == MODE_OMNISHADOW){
		float lightDistance = length(m_positionVarying.xyz - lightPos);
		lightDistance = lightDistance / farPlane;
		fragColor = vec4(lightDistance, 0.0, 0.0, 1.0);
		gl_FragDepth = lightDistance;
	}
}