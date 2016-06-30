#ifndef ofxPBRShadow_h
#define ofxPBRShadow_h

#include "ofMain.h"
#include "ofxPBRLight.h"
#include "shaders/blur.h"

class ofxPBRShadow{
public:
    void setup(int resolution);
    void resizeDepthMap(int resolution);
    void setNumLights(int numLights);
    void beginDepthMap(ofxPBRLight * pbrLight, int index);
    void endDepthMap();
    ofTexture *getDepthMap();
    int getDepthMapResolution();
    ofVec2f getDepthMapAtrasRes();
    ofVec2f getDepthTexMag();
    
private:
    void setShadowMap();
    
    ofShader blurShader, depthShader;
    ofMatrix4x4 inverseCameraMatrix;
    int depthMapRes;
    int depthMapAtrasWidth, depthMapAtrasHeight;
    ofVec2f depthTexMag;
    ofFbo depthMap;
    ofFbo depthSumFbo, blurVFbo, blurHFbo;
    ofFbo::Settings settings;
    ofxPBRLight * pbrLight;
    int numLights;
    Blur blur;

	int currentLightIndex;
};

#endif