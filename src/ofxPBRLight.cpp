#include "ofxPBRLight.h"

ofxPBRLight::ofxPBRLight() {
	this->setupPerspective();
	this->setForceAspectRatio(1.0);
	this->setFov(90);
}

ofxPBRLight::~ofxPBRLight() {
}

void ofxPBRLight::setLightFunction(function<void()> func) {
    resetLights = func;
}

void ofxPBRLight::enable(bool isEnabled) {
	this->lightParams.enable = isEnabled;
}

void ofxPBRLight::disable() {
	this->lightParams.enable = false;
}

bool ofxPBRLight::isEnabled() {
	return lightParams.enable;
}

// depth camera
void ofxPBRLight::setDepthMapRes(float resolution) {
	lightParams.depthMapRes = resolution;
}

// for rendering shader
ofVec3f ofxPBRLight::getViewSpacePosition(ofMatrix4x4 viewMatrix) {
	float w = 1.0;
	if (lightParams.lightType == LightType_Directional || lightParams.lightType == LightType_Sky) {
		w = 0.0;
	}
	ofVec4f pos = ofVec4f(this->getGlobalPosition().x, this->getGlobalPosition().y, this->getGlobalPosition().z, w);
	return pos * viewMatrix;
}

ofMatrix4x4 ofxPBRLight::getShadowMatrix(ofMatrix4x4 cameraModelViewMatrix) {
	ofMatrix4x4 viewMatrix, projectionMatrix;
	viewMatrix = this->getModelViewMatrix();
	if (lightParams.lightType == LightType_Directional || lightParams.lightType == LightType_Sky) {
		projectionMatrix = this->getOrthoMatrix();
	}
	else {
		projectionMatrix = this->getProjectionMatrix();
	}
	return cameraModelViewMatrix.getInverse() * viewMatrix * projectionMatrix * biasMatrix;
}

ofVec3f ofxPBRLight::getViewSpaceDirection(ofMatrix4x4 viewMatrix) {
	ofVec4f dir = ofVec4f(getLookAtDir().x, getLookAtDir().y, getLookAtDir().z, 0.0);
	return ofVec3f(dir * viewMatrix).getNormalized();
}

ofMatrix4x4 ofxPBRLight::getViewProjectionMatrix() {
	ofMatrix4x4 viewMatrix, projectionMatrix;
	viewMatrix = this->getModelViewMatrix();
	if (lightParams.lightType == LightType_Directional || lightParams.lightType == LightType_Sky) {
		projectionMatrix = this->getOrthoMatrix();
	}
	else {
		projectionMatrix = this->getProjectionMatrix();
	}
	return viewMatrix * projectionMatrix;
}

ofMatrix4x4 ofxPBRLight::getViewProjectionMatrix(int face)
{
	return lightParams.pointLight.viewProjMat[face];
}

ofMatrix4x4 ofxPBRLight::getOrthoMatrix()
{
	ofMatrix4x4 mat;
	mat.makeOrthoMatrix(-lightParams.depthMapRes * 0.5, lightParams.depthMapRes * 0.5, -lightParams.depthMapRes * 0.5, lightParams.depthMapRes * 0.5, this->getNearClip(), this->getFarClip());
	return mat;
}

// lightParams.color
void ofxPBRLight::setColor(ofFloatColor color) {
	this->lightParams.color = color;
}

void ofxPBRLight::setColor(ofColor color) {
	this->lightParams.color.r = color.r / 255.0;
	this->lightParams.color.g = color.g / 255.0;
	this->lightParams.color.b = color.b / 255.0;
	this->lightParams.color.a = color.a / 255.0;
}

ofFloatColor ofxPBRLight::getColor() {
	return lightParams.color;
}

// light
void ofxPBRLight::setLightType(LightType lightType) {
	this->lightParams.lightType = lightType;
	switch (lightParams.lightType) {
	case LightType_Directional:
	case LightType_Sky:
		this->enableOrtho();
		break;

	case LightType_Spot:
		this->disableOrtho();
		this->setFov(lightParams.spotLight.cutoff * 2);
		break;

	case LightType_Point:
		this->disableOrtho();
		this->setFov(90);
		break;

	default:
		break;
	}
    resetLights();
}

LightType ofxPBRLight::getLightType() {
	return lightParams.lightType;
}

void ofxPBRLight::setIntensity(float intensity) {
	this->lightParams.intensity = intensity;
}

float ofxPBRLight::getIntensity() {
	return lightParams.intensity;
}

// spotlight & pointlight
void ofxPBRLight::setPointLightRadius(float radius) {
	lightParams.pointLight.radius = radius;
}

float ofxPBRLight::getPointLightRadius() {
	return lightParams.pointLight.radius;
}

// spotLight
void ofxPBRLight::setSpotLightDistance(float distance)
{
	lightParams.spotLight.distance = distance;
}

float ofxPBRLight::getSpotLightDistance()
{
	return lightParams.spotLight.distance;
}

void ofxPBRLight::setSpotLightCutoff(float cutoff) {
	lightParams.spotLight.cutoff = cutoff;
	this->setFov(lightParams.spotLight.cutoff * 2);
}

float ofxPBRLight::getSpotLightCutoff() {
	return lightParams.spotLight.cutoff;
}

void ofxPBRLight::setSpotLightFactor(float spotFactor) {
	lightParams.spotLight.spotFactor = spotFactor;
}

float ofxPBRLight::getSpotLightFactor() {
	return lightParams.spotLight.spotFactor;
}

// shadow
void ofxPBRLight::setShadowType(ShadowType shadowType) {
	this->lightParams.shadowType = shadowType;
}

ShadowType ofxPBRLight::getShadowType() {
	return lightParams.shadowType;
}

void ofxPBRLight::setShadowBias(float shadowBias) {
	this->lightParams.shadowBias = shadowBias;
}

float ofxPBRLight::getShadowBias() {
	return lightParams.shadowBias;
}

void ofxPBRLight::setShadowIndex(int index)
{
	lightParams.shadowIndex = index;
}

int ofxPBRLight::getShadowIndex()
{
	return lightParams.shadowIndex;
}

void ofxPBRLight::setShadowStrength(float strength)
{
	this->lightParams.strength = strength;
}

float ofxPBRLight::getShadowStrength()
{
	return lightParams.strength;
}

void ofxPBRLight::setOmniShadowIndex(int index)
{
	lightParams.pointLight.index = index;
}

int ofxPBRLight::getOmniShadowIndex()
{
	return lightParams.pointLight.index;
}

void ofxPBRLight::updateOmniShadowParams()
{
	lightParams.pointLight.shadowProjMatrix.makePerspectiveMatrix(90.0, 1.0, 1.0, getFarClip());

	lightParams.pointLight.lookAtMat[0].makeLookAtViewMatrix(getGlobalPosition(), getGlobalPosition() + ofVec3f(1, 0, 0), ofVec3f(0, -1, 0));
	lightParams.pointLight.lookAtMat[1].makeLookAtViewMatrix(getGlobalPosition(), getGlobalPosition() + ofVec3f(-1, 0, 0), ofVec3f(0, -1, 0));
	lightParams.pointLight.lookAtMat[2].makeLookAtViewMatrix(getGlobalPosition(), getGlobalPosition() + ofVec3f(0, 1, 0), ofVec3f(0, 0, 1));
	lightParams.pointLight.lookAtMat[3].makeLookAtViewMatrix(getGlobalPosition(), getGlobalPosition() + ofVec3f(0, -1, 0), ofVec3f(0, 0, -1));
	lightParams.pointLight.lookAtMat[4].makeLookAtViewMatrix(getGlobalPosition(), getGlobalPosition() + ofVec3f(0, 0, 1), ofVec3f(0, -1, 0));
	lightParams.pointLight.lookAtMat[5].makeLookAtViewMatrix(getGlobalPosition(), getGlobalPosition() + ofVec3f(0, 0, -1), ofVec3f(0, -1, 0));

	// make view projection matricies
	for (int i = 0; i < 6; i++) {
		lightParams.pointLight.viewProjMat[i] = lightParams.pointLight.lookAtMat[i] * lightParams.pointLight.shadowProjMatrix;
	}
}

void ofxPBRLight::beginLighting(ofShader * shader) {
	string lightIndex = ofToString(lightParams.lightId);
	shader->setUniform1i("lights[" + lightIndex + "].isEnabled", isEnabled());
	shader->setUniform3f("lights[" + lightIndex + "].position", getGlobalPosition());
	shader->setUniform3f("lights[" + lightIndex + "].vPosition", getViewSpacePosition(ofGetCurrentViewMatrix()));
	shader->setUniform3f("lights[" + lightIndex + "].direction", getViewSpaceDirection(ofGetCurrentViewMatrix()));
	shader->setUniform1i("lights[" + lightIndex + "].type", getLightType());
	shader->setUniform1i("lights[" + lightIndex + "].shadowType", getShadowType());
	shader->setUniform1f("lights[" + lightIndex + "].intensity", getIntensity());
	shader->setUniform1i("lights[" + lightIndex + "].shadowIndex", getShadowIndex());
	shader->setUniform1i("lights[" + lightIndex + "].index", getOmniShadowIndex());
	shader->setUniform1f("lights[" + lightIndex + "].shadowStrength", getShadowStrength());
	shader->setUniform1f("lights[" + lightIndex + "].farClip", getFarClip());
	shader->setUniform1f("lights[" + lightIndex + "].bias", getShadowBias());

	float exposure = 1.0;
	switch (lightParams.lightType)
	{
	case LightType_Sky:
		exposure = lightParams.skyLight.exposure;
		break;

	case LightType_Point:
		shader->setUniform1f("lights[" + lightIndex + "].pointLightRadius", getPointLightRadius());
		break;

	case LightType_Spot:
		shader->setUniform1f("lights[" + lightIndex + "].spotLightFactor", getSpotLightFactor());
		shader->setUniform1f("lights[" + lightIndex + "].spotLightCutoff", getSpotLightCutoff());
        shader->setUniform1f("lights[" + lightIndex + "].spotLightDistance", getSpotLightDistance());
		break;

	default:
		break;
	}
	ofVec4f color = ofVec4f(getColor().r * exposure, getColor().g * exposure, getColor().b * exposure, 1.0);
	shader->setUniform4f("lights[" + lightIndex + "].color", color);
}

void ofxPBRLight::endLighting(ofShader * shader) {
	shader->setUniform1i("lights[" + ofToString(lightParams.lightId) + "].isEnabled", 0);
}

// Sky light settings
void ofxPBRLight::setSkyLightCoordinate(float longitude, float latitude, float radius)
{
	lightParams.skyLight.latitude = latitude;
	lightParams.skyLight.longitude = longitude;
	lightParams.skyLight.radius = radius;
	setSkyLightPos();
}

void ofxPBRLight::setSkyLightRotation(float angle)
{
	lightParams.skyLight.angle = angle;
	setSkyLightPos();
}

void ofxPBRLight::setSkyLightPos()
{
	ofVec3f pos = ofVec3f::zero();
	pos.x = sin(lightParams.skyLight.latitude) * cos(lightParams.skyLight.longitude + lightParams.skyLight.angle);
	pos.z = sin(lightParams.skyLight.latitude) * sin(lightParams.skyLight.longitude + lightParams.skyLight.angle);
	pos.y = cos(lightParams.skyLight.latitude);
	pos *= lightParams.skyLight.radius;
	setPosition(pos);
	this->lookAt(ofVec3f::zero());
}

void ofxPBRLight::setSkyLighExposure(float exposure)
{
	lightParams.skyLight.exposure = exposure;
}

float ofxPBRLight::getSkyLightLatitude()
{
	return lightParams.skyLight.latitude;
}

float ofxPBRLight::getSkyLightLongitude()
{
	return lightParams.skyLight.longitude;
}

float ofxPBRLight::getSkyLightRadius()
{
	return lightParams.skyLight.radius;
}

void ofxPBRLight::setId(int lightId)
{
	this->lightParams.lightId = lightId;
}

int ofxPBRLight::getId()
{
	return lightParams.lightId;
}

ofxPBRLightParams ofxPBRLight::getParameters()
{
	return lightParams;
}

void ofxPBRLight::setParameters(ofxPBRLightParams params)
{
	lightParams = params;
	setLightType(lightParams.lightType);
	if (lightParams.lightType == LightType_Sky) {
		setSkyLightPos();
	}
}
