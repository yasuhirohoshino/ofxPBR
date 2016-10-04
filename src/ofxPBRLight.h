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
    
    void setLightFunction(function<void()> func);
    
    void enable(bool isEnabled = true);
    void disable();
    bool isEnabled();
    
    // depth camera
    void lookAt(ofVec3f target);
    void setDepthMapRes(float resolution);
    
    // for rendering shader
    ofVec3f getViewSpacePosition(ofMatrix4x4 viewMatrix);
    ofMatrix4x4 getShadowMatrix(ofMatrix4x4 cameraModelViewMatrix);
    ofVec3f getViewSpaceDirection(ofMatrix4x4 viewMatrix);
    ofMatrix4x4 getViewProjectionMatrix();
	ofMatrix4x4 getViewProjectionMatrix(int face);
    
    // color
    void setColor(ofFloatColor color);
    void setColor(ofColor color);
    ofFloatColor getColor();
	void setIntensity(float intensity);
	float getIntensity();
    
    // light type
    void setLightType(LightType lightType);
    LightType getLightType();
    
    // pointLight
    void setPointLightRadius(float radius);
    float getPointLightRadius();
    
    // spotLight
	void setSpotLightDistance(float distance);
	float getSpotLightDistance();
    void setSpotLightCutoff(float cutoff);
    float getSpotLightCutoff();
    void setSpotLightFactor(float spotFactor);
    float getSpotLightFactor();

	// skyLight
	void setSkyLightCoordinate(float longitude, float latitude, float radius);
	void setSkyLightRotation(float angle);
	void setSkyLighExposure(float exposure);
	float getSkyLightLatitude();
	float getSkyLightLongitude();
	float getSkyLightRadius();
    
    // shadow
    void setShadowType(ShadowType shadowType);
    ShadowType getShadowType();
    void setShadowBias(float shadowBias);
    float getShadowBias();
	void setShadowIndex(int index);
	int getShadowIndex();
	void setShadowStrength(float strength);
	float getShadowStrength();
	void setOmniShadowIndex(int index);
	int getOmniShadowIndex();
	void updateOmniShadowParams();

    void beginLighting(ofShader * shader);
    void endLighting(ofShader * shader);

	void setId(int id);
	int getId();
    
private:
	//void begin() { this->::ofCamera::begin(); };
	//void end() { this->::ofCamera::end(); };
	void setSkyLightPos();
	ofMatrix4x4 getOrthoMatrix();
    
    struct parameters{
        ofFloatColor color = ofFloatColor(1.0,1.0,1.0,1.0);
        float intensity = 1.0;
        float depthMapRes = 1024;
        float shadowBias = 0.001;
        ofMatrix4x4 shadowTransMatrix;
    };
    
    struct SpotLightParams{
        float spotFactor = 1;
        float cutoff = 45;
        float distance = 1000;
    };
    
    struct PointLightParams{
        float radius = 1000;
    };
    
    struct SkyLightParams{
        float latitude = 0;
        float longitude = 0;
        float radius = 0;
        float exposure = 1.0;
        float angle = 0;
    };

	struct OmniShadowParams {
		int omniShadowIndex = 0;
		ofMatrix4x4 viewProjMat[6];
		ofMatrix4x4 lookAtMat[6];
		ofMatrix4x4 shadowProjMatrix;
	};

	SpotLightParams spotLightParams;
	PointLightParams pointLightParams;
	SkyLightParams skyLightParams;
	OmniShadowParams omniShadowParams;

	// common
	bool isLightEnabled = true;
	string lightIndex;
	int lightId;
	LightType lightType = LightType_Directional;

	// color
    ofFloatColor color = ofFloatColor(1.0,1.0,1.0,1.0);
	float intensity = 1.0;

	// target
	ofVec3f target = ofVec3f::zero();

	// shadow
	ShadowType shadowType = ShadowType_Hard;
	int shadowIndex;
    float depthMapRes = 1024;
    float shadowBias = 0.0005;
	float strength = 1.0;

	ofMatrix4x4 shadowTransMatrix;
    const ofMatrix4x4 biasMatrix = ofMatrix4x4(
                                               0.5, 0.0, 0.0, 0.0,
                                               0.0, 0.5, 0.0, 0.0,
                                               0.0, 0.0, 0.5, 0.0,
                                               0.5, 0.5, 0.5, 1.0
                                               );
    function<void()> resetLights;
};