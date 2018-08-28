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

struct ofxPBRLightData{
    LightType lightType = LightType_Directional;
	bool enable = true;
    ofFloatColor color = ofFloatColor(1.0,1.0,1.0,1.0);
    float intensity = 1.0;

	// shadow
	ShadowType shadowType = ShadowType_Hard;
	float shadowBias = 0.005;
	float shadowStrength = 1.0;

	// pointLight
	float pointLightRadius = 1000;

	// spotLight
	float spotLightGradient = 1;
	float spotLightCutoff = 45;
	float spotLightDistance = 5000;

	// skyLight
	float skyLightLatitude = 0;
	float skyLightLongitude = 0;
	float skyLightAngle = 0;
};

class ofxPBRLight : public ofCamera{
public:
    ofxPBRLight();
    ~ofxPBRLight();

	void setup();
    void setEnable(bool enable);
	void setLightType(LightType lightType);

    // color
    void setColor(ofFloatColor color);
    void setColor(ofColor color);
	void setIntensity(float intensity);

	// pointLight
	void setPointLightRadius(float radius);
    
    // spotLight
	void setSpotLightCutoff(float cutoff);
	void setSpotLightGradient(float spotFactor);
	void setSpotLightDistance(float distance);

	// skyLight
	void setSkyLightCoordinate(float longitude, float latitude);
	void setSkyLightRotation(float angle);
    
    // shadow
    void setShadowType(ShadowType shadowType);
    void setShadowBias(float shadowBias);
	void setShadowStrength(float strength);
	void setSpotShadowIndex(int index);
	void setOmniShadowIndex(int index);
	void setDirectionalShadowIndex(int index);

	// paremeters
	void setParameters(ofxPBRLightData params);

	// getter

	bool getIsEnabled() { return lightData.enable; }
	LightType getLightType() { return lightData.lightType; }

	ofFloatColor getColor() { return lightData.color; }
	float getIntensity() { return lightData.intensity; }

	float getPointLightRadius() { return lightData.pointLightRadius; }

	float getSpotLightCutoff() { return lightData.spotLightCutoff; }
	float getSpotLightGradient() { return lightData.spotLightGradient; }
	float getSpotLightDistance() { return lightData.spotLightDistance; }

	float getSkyLightLatitude() { return lightData.skyLightLatitude; }
	float getSkyLightLongitude() { return lightData.skyLightLongitude; }

	ShadowType getShadowType() { return lightData.shadowType; }
	float getShadowBias() { return lightData.shadowBias; }
	float getShadowStrength() { return lightData.shadowStrength; }
	int getSpotShadowIndex() { return spotShadowIndex; }
	int getOmniShadowIndex() { return pointLightIndex; }
	int getDirectionalShadowIndex() { return directionalShadowIndex; }

	ofxPBRLightData getParameters() { return lightData; }

	// for rendering shader
	ofVec3f getViewSpacePosition(glm::mat4 viewMatrix);
	ofVec3f getViewSpaceDirection(glm::mat4 viewMatrix);

private:
	void setSkyLightPos();
	ofxPBRLightData lightData;

	int spotShadowIndex = 0;
	int pointLightIndex = 0;
	int directionalShadowIndex = 0;
};