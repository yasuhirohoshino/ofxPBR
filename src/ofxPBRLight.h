#pragma once

#include "ofMain.h"

enum LightType{
    LightType_Directional = 0,
    LightType_Spot = 1,
    LightType_Point = 2,
	LightType_Sky = 3,
    NumLightTypes = 4
};

enum ShadowType{
    ShadowType_None = 0,
    ShadowType_Hard = 1,
    ShadowType_Soft = 2,
    NumShadowTypes = 3
};

class ofxPBRLight : public ofCamera{
public:
    ofxPBRLight();
    ~ofxPBRLight();

	void setup(float resolution);
    
    void enable(bool isEnabled = true);
    void disable();
    bool isEnabled();
	void setSkyLightCoordinate(float longitude, float latitude, float radius);
	void setSkyLightRotation(float angle);
    
    // depth camera
    void lookAt(ofVec3f target);
    void setDepthMapRes(float resolution);
    void beginDepthCamera();
    void endDepthCamera();
    
    // for rendering shader
    ofVec3f getViewSpacePosition(ofMatrix4x4 viewMatrix);
    ofMatrix4x4 getShadowMatrix(ofMatrix4x4 cameraModelViewMatrix);
    ofVec3f getViewSpaceDirection(ofMatrix4x4 viewMatrix);
    ofMatrix4x4 getViewProjectionMatrix();
    
    // color
    void setColor(ofFloatColor color);
    void setColor(ofColor color);
    ofFloatColor getColor();
    
    // light
    void setLightType(LightType lightType);
    LightType getLightType();
    void setIntensity(float intensity);
    float getIntensity();
    
    // spotlight & pointlight
    void setRadius(float radius);
    float getRadius();
    
    // spotLight
    void setCutoff(float cutoff);
    float getCutoff();
    void setSpotFactor(float spotFactor);
    float getSpotFactor();
    
    // shadow
    void setShadowType(ShadowType shadowType);
    ShadowType getShadowType();
    void setShadowBias(float shadowBias);
    float getShadowBias();
    void setSoftShadowExponent(float softShadowExponent);
    float getSoftShadowExponent();
    void beginLighting(ofShader * shader, int index);
    void endLighting(ofShader * shader);
    
	void setSkyLighExposure(float brightness);

	float getSkyLightLatitude();
	float getSkyLightLongitude();
	float getSkyLightRadius();

	void setId(int id);
	int getId();
    
private:
	void begin() { this->::ofCamera::begin(); };
	void end() { this->::ofCamera::end(); };
    ofFloatColor color = ofFloatColor(1.0,1.0,1.0,1.0);
    LightType lightType = LightType_Directional;
    ShadowType shadowType = ShadowType_Soft;
	ofVec3f target = ofVec3f::zero();
    float spotFactor = 1;
    float cutoff = 45;
    float radius = 1000;
    float depthMapRes = 1024;
    float shadowBias = 0.001;
    float softShadowExponent = 75.0;
    float intensity = 1.0;
    bool isLightEnabled = true;
    ofMatrix4x4 shadowTransMatrix;
    const ofMatrix4x4 biasMatrix = ofMatrix4x4(
                                               0.5, 0.0, 0.0, 0.0,
                                               0.0, 0.5, 0.0, 0.0,
                                               0.0, 0.0, 0.5, 0.0,
                                               0.5, 0.5, 0.5, 1.0
                                               );
	string lightIndex;
	int lightId;

	float skyLightLatitude = 0;
	float skyLightLongitude = 0;
	float skyLightRadius = 0;
	float skyLightExposure = 1.0;
	float skyLightAngle = 0;
};