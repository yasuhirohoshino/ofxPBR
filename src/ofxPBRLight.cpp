#include "ofxPBRLight.h"

ofxPBRLight::ofxPBRLight() {
	this->setupPerspective();
	this->setForceAspectRatio(1.0);
	this->setFov(90);
}

ofxPBRLight::~ofxPBRLight() {
}

void ofxPBRLight::enable(bool isEnabled) {
	this->isLightEnabled = isEnabled;
}

void ofxPBRLight::disable() {
	this->isLightEnabled = false;
}

bool ofxPBRLight::isEnabled() {
	return isLightEnabled;
}

// depth camera
void ofxPBRLight::lookAt(ofVec3f target) {
	this->target = target;
	this->ofCamera::lookAt(this->target);
}

void ofxPBRLight::setDepthMapRes(float resolution) {
	depthMapRes = resolution;
}

void ofxPBRLight::beginDepthCamera() {
	this->begin();
}

void ofxPBRLight::endDepthCamera() {
	this->end();
}

// for rendering shader
ofVec3f ofxPBRLight::getViewSpacePosition(ofMatrix4x4 viewMatrix) {
	float w = 1.0;
	if (lightType == LightType_Directional || lightType == LightType_Sky) {
		w = 0.0;
	}
	ofVec4f pos = ofVec4f(this->getGlobalPosition().x, this->getGlobalPosition().y, this->getGlobalPosition().z, w);
	return pos * viewMatrix;
}

ofMatrix4x4 ofxPBRLight::getShadowMatrix(ofMatrix4x4 cameraModelViewMatrix) {
	ofMatrix4x4 viewMatrix, projectionMatrix;
	viewMatrix = this->getModelViewMatrix();
	if (lightType == LightType_Directional || lightType == LightType_Sky) {
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
	if (lightType == LightType_Directional || lightType == LightType_Sky) {
		projectionMatrix = this->getOrthoMatrix();
	}
	else {
		projectionMatrix = this->getProjectionMatrix();
	}
	return viewMatrix * projectionMatrix;
}

// color
void ofxPBRLight::setColor(ofFloatColor color) {
	this->color = color;
}

void ofxPBRLight::setColor(ofColor color) {
	this->color.r = color.r / 255.0;
	this->color.g = color.g / 255.0;
	this->color.b = color.b / 255.0;
	this->color.a = color.a / 255.0;
}

ofFloatColor ofxPBRLight::getColor() {
	return color;
}

// light
void ofxPBRLight::setLightType(LightType lightType) {
	this->lightType = lightType;
	if (lightType == LightType_Directional || lightType == LightType_Sky) {
		this->enableOrtho();
	}
	else {
		this->disableOrtho();
	}
	switch (lightType) {
	case LightType_Directional:
	case LightType_Sky:
		this->enableOrtho();
		break;

	case LightType_Spot:
		this->disableOrtho();
		this->setFov(spotLightParams.cutoff * 2);
		break;

	case LightType_Point:
		this->disableOrtho();
		this->setFov(90);
		break;

	default:
		break;
	}
}

LightType ofxPBRLight::getLightType() {
	return lightType;
}

void ofxPBRLight::setIntensity(float intensity) {
	this->intensity = intensity;
}

float ofxPBRLight::getIntensity() {
	return intensity;
}

// spotlight & pointlight
void ofxPBRLight::setRadius(float radius) {
	pointLightParams.radius = radius;
}

float ofxPBRLight::getRadius() {
	return pointLightParams.radius;
}

// spotLight
void ofxPBRLight::setSpotLightDistance(float distance)
{
	spotLightParams.distance = distance;
}

float ofxPBRLight::getSpotLightDistance()
{
	return spotLightParams.distance;
}

void ofxPBRLight::setSpotLightCutoff(float cutoff) {
	spotLightParams.cutoff = cutoff;
	this->setFov(spotLightParams.cutoff * 2);
}

float ofxPBRLight::getSpotLightCutoff() {
	return spotLightParams.cutoff;
}

void ofxPBRLight::setSpotLightFactor(float spotFactor) {
	spotLightParams.spotFactor = spotFactor;
}

float ofxPBRLight::getSpotLightFactor() {
	return spotLightParams.spotFactor;
}

// shadow
void ofxPBRLight::setShadowType(ShadowType shadowType) {
	this->shadowType = shadowType;
}

ShadowType ofxPBRLight::getShadowType() {
	return shadowType;
}

void ofxPBRLight::setShadowBias(float shadowBias) {
	this->shadowBias = shadowBias;
}

float ofxPBRLight::getShadowBias() {
	return shadowBias;
}

void ofxPBRLight::setSoftShadowExponent(float softShadowExponent) {
	this->softShadowExponent = softShadowExponent;
}

float ofxPBRLight::getSoftShadowExponent() {
	return softShadowExponent;
}

void ofxPBRLight::beginLighting(ofShader * shader, int index) {
	lightId = index;
	string lightIndex = ofToString(lightId);
	shader->setUniform1i("lights[" + lightIndex + "].isEnabled", isEnabled());
	shader->setUniform3f("lights[" + lightIndex + "].position", getViewSpacePosition(ofGetCurrentViewMatrix()));
	shader->setUniform3f("lights[" + lightIndex + "].direction", getViewSpaceDirection(ofGetCurrentViewMatrix()));
	shader->setUniform1i("lights[" + lightIndex + "].type", getLightType());
	shader->setUniform1i("lights[" + lightIndex + "].shadowType", getShadowType());
	shader->setUniform1f("lights[" + lightIndex + "].intensity", getIntensity());
	shader->setUniform1f("lights[" + lightIndex + "].softShadowExponent", getSoftShadowExponent());
	shader->setUniform1f("lights[" + lightIndex + "].bias", getShadowBias());

	float exposure = 1.0;
	switch (lightType)
	{
	case LightType_Sky:
		exposure = skyLightParams.exposure;
		break;

	case LightType_Point:
		shader->setUniform1f("lights[" + lightIndex + "].pointLightRadius", getRadius());
		break;

	case LightType_Spot:
		shader->setUniform1f("lights[" + lightIndex + "].spotLightFactor", getSpotLightFactor());
		shader->setUniform1f("lights[" + lightIndex + "].spotLightCutoff", getSpotLightCutoff());
		break;

	default:
		break;
	}
	ofVec4f color = ofVec4f(getColor().r * exposure, getColor().g * exposure, getColor().b * exposure, 1.0);
	shader->setUniform4f("lights[" + lightIndex + "].color", color);
}

void ofxPBRLight::endLighting(ofShader * shader) {
	shader->setUniform1i("lights[" + lightIndex + "].isEnabled", 0);
}

// Sky light settings

void ofxPBRLight::setSkyLightCoordinate(float longitude, float latitude, float radius)
{
	skyLightParams.latitude = latitude;
	skyLightParams.longitude = longitude;
	skyLightParams.radius = radius;
	setSkyLightPos();
}

void ofxPBRLight::setSkyLightRotation(float angle)
{
	skyLightParams.angle = angle;
	setSkyLightPos();
}

void ofxPBRLight::setSkyLightPos()
{
	ofVec3f pos = ofVec3f::zero();
	pos.x = sin(skyLightParams.latitude) * cos(skyLightParams.longitude + skyLightParams.angle);
	pos.z = sin(skyLightParams.latitude) * sin(skyLightParams.longitude + skyLightParams.angle);
	pos.y = cos(skyLightParams.latitude);
	pos *= skyLightParams.radius;
	setPosition(pos);
	this->lookAt(ofVec3f::zero());
}

ofMatrix4x4 ofxPBRLight::getOrthoMatrix()
{
	ofMatrix4x4 mat;
	mat.makeOrthoMatrix(-depthMapRes * 0.5, depthMapRes * 0.5, -depthMapRes * 0.5, depthMapRes * 0.5, this->getNearClip(), this->getFarClip());
	return mat;
}

void ofxPBRLight::setSkyLighExposure(float exposure)
{
	skyLightParams.exposure = exposure;
}

float ofxPBRLight::getSkyLightLatitude()
{
	return skyLightParams.latitude;
}

float ofxPBRLight::getSkyLightLongitude()
{
	return skyLightParams.longitude;
}

float ofxPBRLight::getSkyLightRadius()
{
	return skyLightParams.radius;
}

void ofxPBRLight::setId(int lightId)
{
	this->lightId = lightId;
}

int ofxPBRLight::getId()
{
	return lightId;
}