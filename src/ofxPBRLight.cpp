#include "ofxPBRLight.h"

ofxPBRLight::ofxPBRLight() {}
ofxPBRLight::~ofxPBRLight() {}

void ofxPBRLight::setup() {
	this->setupPerspective(false, 90, 0.0, 10000);
	this->setForceAspectRatio(1.0);
	this->setFov(90);
	this->enableOrtho();
}

void ofxPBRLight::setEnable(bool enable) {
	this->lightData.enable = enable;
}

// for rendering shader

ofVec3f ofxPBRLight::getViewSpacePosition(glm::mat4 viewMatrix) {
	float w = 1.0;
	if (lightData.lightType == LightType_Directional || lightData.lightType == LightType_Sky) {
		w = 0.0;
	}
	ofVec4f pos = ofVec4f(this->getPosition().x, this->getPosition().y, this->getPosition().z, w);
	return  viewMatrix * pos;
}

ofVec3f ofxPBRLight::getViewSpaceDirection(glm::mat4 viewMatrix) {
	ofVec4f dir = ofVec4f(getLookAtDir().x, getLookAtDir().y, getLookAtDir().z, 0.0);
	return ofVec3f(viewMatrix * dir).getNormalized();
}

// color

void ofxPBRLight::setColor(ofFloatColor color) {
	this->lightData.color = color;
}

void ofxPBRLight::setColor(ofColor color) {
	this->lightData.color.r = color.r / 255.0;
	this->lightData.color.g = color.g / 255.0;
	this->lightData.color.b = color.b / 255.0;
	this->lightData.color.a = color.a / 255.0;
}

void ofxPBRLight::setIntensity(float intensity) {
	this->lightData.intensity = intensity;
}

void ofxPBRLight::setPointLightRadius(float radius)
{
	lightData.pointLightRadius = radius;
}

// light

void ofxPBRLight::setLightType(LightType lightType) {
	this->lightData.lightType = lightType;
	switch (lightData.lightType) {
	case LightType_Directional:
	case LightType_Sky:
		this->enableOrtho();
		break;

	case LightType_Spot:
		this->disableOrtho();
		this->setFov(lightData.spotLightCutoff * 2);
		break;

	case LightType_Point:
		this->disableOrtho();
		this->setFov(90);
		break;

	default:
		break;
	}
}

void ofxPBRLight::setSpotLightDistance(float distance)
{
	lightData.spotLightDistance = distance;
	setFarClip(distance);
	setNearClip(distance * 0.05);
}

void ofxPBRLight::setSpotLightCutoff(float cutoff) {
	lightData.spotLightCutoff = cutoff;
	this->setFov(lightData.spotLightCutoff * 2);
}

void ofxPBRLight::setSpotLightGradient(float spotLightGradient) {
	lightData.spotLightGradient = spotLightGradient;
}

// sky light settings

void ofxPBRLight::setSkyLightCoordinate(float longitude, float latitude)
{
	lightData.skyLightLongitude = longitude;
	lightData.skyLightLatitude = latitude;
	setSkyLightPos();
}

void ofxPBRLight::setSkyLightRotation(float angle)
{
	lightData.skyLightAngle = angle;
	setSkyLightPos();
}

void ofxPBRLight::setSkyLightPos()
{
	ofVec3f pos = ofVec3f::zero();
	pos.x = sin(lightData.skyLightLatitude) * cos(lightData.skyLightLongitude + lightData.skyLightAngle);
	pos.z = sin(lightData.skyLightLatitude) * sin(lightData.skyLightLongitude + lightData.skyLightAngle);
	pos.y = cos(lightData.skyLightLatitude);
	setPosition(pos);
	this->lookAt(ofVec3f::zero(), glm::vec3(0, 1, 0));
}

// shadow

void ofxPBRLight::setShadowType(ShadowType shadowType) {
	this->lightData.shadowType = shadowType;
}

void ofxPBRLight::setShadowBias(float shadowBias) {
	this->lightData.shadowBias = shadowBias;
}

void ofxPBRLight::setSpotShadowIndex(int index)
{
	spotShadowIndex = index;
}

void ofxPBRLight::setShadowStrength(float strength)
{
	this->lightData.shadowStrength = strength;
}

void ofxPBRLight::setOmniShadowIndex(int index)
{
	pointLightIndex = index;
}

void ofxPBRLight::setDirectionalShadowIndex(int index)
{
	directionalShadowIndex = index;
}

// light paremeters

void ofxPBRLight::setParameters(ofxPBRLightData params)
{
	lightData = params;
	setLightType(lightData.lightType);
	if (lightData.lightType == LightType_Sky) {
		setSkyLightPos();
	}
}