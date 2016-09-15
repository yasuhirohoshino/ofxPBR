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
    void beginDepthMap();
    void endDepthMap();
    ofTexture *getDepthMap();
    int getDepthMapResolution();
    ofVec2f getDepthMapAtrasRes();
    ofVec2f getDepthTexMag();
    
    void bind(GLuint index);
    void unbind();
    
private:
    void setShadowMap();
    
    ofShader blurShader;
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
    
    GLuint depthMapIndex, depthMapFbo;
};

#endif