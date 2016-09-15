#include "ofxPBRLight.h"

ofxPBRLight::ofxPBRLight(){
    this->setupPerspective();
    this->setForceAspectRatio(1.0);
    this->setFov(90);
}

ofxPBRLight::~ofxPBRLight(){
}

void ofxPBRLight::enable(bool isEnabled){
    this->isLightEnabled = isEnabled;
}

void ofxPBRLight::disable(){
    this->isLightEnabled = false;
}

bool ofxPBRLight::isEnabled(){
    return isLightEnabled;
}

void ofxPBRLight::setSkyLightCoordinate(float longitude, float latitude, float radius)
{
	skyLightLatitude = latitude;
	skyLightLongitude = longitude;
	skyLightRadius = radius;

	ofVec3f pos = ofVec3f::zero();
	pos.x = sin(skyLightLatitude) * cos(skyLightLongitude + skyLightAngle);
	pos.z = sin(skyLightLatitude) * sin(skyLightLongitude + skyLightAngle);
	pos.y = cos(skyLightLatitude);
	pos *= skyLightRadius;

	setPosition(pos);
	this->lookAt(ofVec3f::zero());
}

void ofxPBRLight::setSkyLightRotation(float angle)
{
	skyLightAngle = angle;
	ofVec3f pos = ofVec3f::zero();
	pos.x = sin(skyLightLatitude) * cos(skyLightLongitude + skyLightAngle);
	pos.z = sin(skyLightLatitude) * sin(skyLightLongitude + skyLightAngle);
	pos.y = cos(skyLightLatitude);
	pos *= skyLightRadius;

	setPosition(pos);
	this->lookAt(ofVec3f::zero());
}

// depth camera
void ofxPBRLight::lookAt(ofVec3f target){
	this->target = target;
	this->ofCamera::lookAt(this->target);
}

void ofxPBRLight::setDepthMapRes(float resolution){
    depthMapRes = resolution;
}

void ofxPBRLight::beginDepthCamera(){
	this->begin();
}

void ofxPBRLight::endDepthCamera(){
	this->end();
}

// for rendering shader
ofVec3f ofxPBRLight::getViewSpacePosition(ofMatrix4x4 viewMatrix){
    if(lightType == LightType_Directional || lightType == LightType_Sky){
        ofVec4f pos = ofVec4f(this->getGlobalPosition().x, this->getGlobalPosition().y, this->getGlobalPosition().z, 0.0);
        return pos * viewMatrix;
    }else{
        ofVec4f pos = ofVec4f(this->getGlobalPosition().x, this->getGlobalPosition().y, this->getGlobalPosition().z, 1.0);
        return pos * viewMatrix;
    }
}

ofMatrix4x4 ofxPBRLight::getShadowMatrix(ofMatrix4x4 cameraModelViewMatrix){
    ofMatrix4x4 viewMatrix, projectionMatrix;
    viewMatrix = this->getModelViewMatrix();
    if(lightType == LightType_Directional || lightType == LightType_Sky){
        projectionMatrix.makeOrthoMatrix(-depthMapRes * 0.5, depthMapRes * 0.5, -depthMapRes * 0.5, depthMapRes * 0.5, this->getNearClip(), this->getFarClip());
    }else{
        projectionMatrix = this->getProjectionMatrix();
    }
    return cameraModelViewMatrix.getInverse() * viewMatrix * projectionMatrix * biasMatrix;
}

ofVec3f ofxPBRLight::getViewSpaceDirection(ofMatrix4x4 viewMatrix){
    ofVec4f dir = ofVec4f(getLookAtDir().x, getLookAtDir().y, getLookAtDir().z, 0.0);
    return ofVec3f(dir * viewMatrix).getNormalized();
}

ofMatrix4x4 ofxPBRLight::getViewProjectionMatrix(){
    ofMatrix4x4 viewMatrix, projectionMatrix;
    viewMatrix = this->getModelViewMatrix();
    if(lightType == LightType_Directional || lightType == LightType_Sky){
        projectionMatrix.makeOrthoMatrix(-depthMapRes * 0.5, depthMapRes * 0.5, -depthMapRes * 0.5, depthMapRes * 0.5, this->getNearClip(), this->getFarClip());
    }else{
        projectionMatrix = this->getProjectionMatrix();
    }
    return viewMatrix * projectionMatrix;
}

// color
void ofxPBRLight::setColor(ofFloatColor color){
    this->color = color;
}

void ofxPBRLight::setColor(ofColor color){
    this->color.r = color.r / 255.0;
    this->color.g = color.g / 255.0;
    this->color.b = color.b / 255.0;
    this->color.a = color.a / 255.0;
}

ofFloatColor ofxPBRLight::getColor(){
    return color;
}

// light
void ofxPBRLight::setLightType(LightType lightType){
    this->lightType = lightType;
    if(lightType == LightType_Directional || lightType == LightType_Sky){
		this->enableOrtho();
    }else{
		this->disableOrtho();
    }
    switch (lightType) {
        case LightType_Directional:
        case LightType_Sky:
            this->enableOrtho();
            break;
            
        case LightType_Spot:
            this->disableOrtho();
            this->setFov(cutoff * 2);
            break;
            
        case LightType_Point:
            this->disableOrtho();
            this->setFov(90);
            break;
            
        default:
            break;
    }
}

LightType ofxPBRLight::getLightType(){
    return lightType;
}

void ofxPBRLight::setIntensity(float intensity){
    this->intensity = intensity;
}

float ofxPBRLight::getIntensity(){
    return intensity;
}

// spotlight & pointlight
void ofxPBRLight::setRadius(float radius){
    this->radius = radius;
}

float ofxPBRLight::getRadius(){
    return radius;
}

// spotLight
void ofxPBRLight::setCutoff(float cutoff){
    this->cutoff = cutoff;
    this->setFov(cutoff * 2);
}

float ofxPBRLight::getCutoff(){
    return cutoff;
}

void ofxPBRLight::setSpotFactor(float spotFactor){
    this->spotFactor = spotFactor;
}

float ofxPBRLight::getSpotFactor(){
    return spotFactor;
}

// shadow
void ofxPBRLight::setShadowType(ShadowType shadowType){
    this->shadowType = shadowType;
}

ShadowType ofxPBRLight::getShadowType(){
    return shadowType;
}

void ofxPBRLight::setShadowBias(float shadowBias){
    this->shadowBias = shadowBias;
}

float ofxPBRLight::getShadowBias(){
    return shadowBias;
}

void ofxPBRLight::setSoftShadowExponent(float softShadowExponent){
    this->softShadowExponent = softShadowExponent;
}

float ofxPBRLight::getSoftShadowExponent(){
    return softShadowExponent;
}

void ofxPBRLight::beginLighting(ofShader * shader, int index){
	lightId = index;
    string lightIndex = ofToString(lightId);
    shader->setUniform1i("lights["+ lightIndex +"].isEnabled", isEnabled());
    shader->setUniform3f("lights["+ lightIndex +"].position", getViewSpacePosition(ofGetCurrentViewMatrix()));
	if (lightType == LightType_Sky) {
		ofVec4f c = ofVec4f(getColor().r * skyLightExposure, getColor().g * skyLightExposure, getColor().b * skyLightExposure, 1.0);
		shader->setUniform4f("lights[" + lightIndex + "].color", c);
	}else{
		shader->setUniform4f("lights[" + lightIndex + "].color", getColor());
	}
    shader->setUniform3f("lights["+ lightIndex +"].direction", getViewSpaceDirection(ofGetCurrentViewMatrix()));
    shader->setUniform1i("lights["+ lightIndex +"].type", getLightType());
    shader->setUniform1i("lights["+ lightIndex +"].shadowType", getShadowType());
    shader->setUniform1f("lights["+ lightIndex +"].intensity", getIntensity());
    shader->setUniform1f("lights["+ lightIndex +"].spotFactor", getSpotFactor());
    shader->setUniform1f("lights["+ lightIndex +"].cutoff", getCutoff());
    shader->setUniform1f("lights["+ lightIndex +"].radius", getRadius());
    shader->setUniform1f("lights["+ lightIndex +"].softShadowExponent", getSoftShadowExponent());
    shader->setUniform1f("lights["+ lightIndex +"].bias", getShadowBias());
}

void ofxPBRLight::endLighting(ofShader * shader){
    shader->setUniform1i("lights["+ lightIndex +"].isEnabled", 0);
}

void ofxPBRLight::setSkyLighExposure(float brightness)
{
	skyLightExposure = brightness;
}

float ofxPBRLight::getSkyLightLatitude()
{
	return skyLightLatitude;
}

float ofxPBRLight::getSkyLightLongitude()
{
	return skyLightLongitude;
}

float ofxPBRLight::getSkyLightRadius()
{
	return skyLightRadius;
}

void ofxPBRLight::setId(int lightId)
{
	this->lightId = lightId;
}

int ofxPBRLight::getId()
{
	return lightId;
}
