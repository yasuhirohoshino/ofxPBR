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
    int getDepthMapResolution();
    ofVec2f getDepthMapAtrasRes();
    ofVec2f getDepthTexMag();
    
    void bind(GLuint index);
    void unbind();
    
private:
    void setShadowMap();
    void setOmniShadowMap(int numOmniShadowMaps);
    
    ofShader blurShader;
    ofMatrix4x4 inverseCameraMatrix;
    int depthMapRes;
    int depthMapAtrasWidth, depthMapAtrasHeight;
    ofVec2f depthTexMag;
    ofFbo depthMap;
    ofFbo blurVFbo, blurHFbo;
    ofFbo::Settings settings;
    ofxPBRLight * pbrLight;
    int numLights;
    Blur blur;

	int currentLightIndex;
    
    GLuint depthMapIndex, depthMapFbo;
    vector<GLuint> depthCubeMapIndex;
    vector<GLuint> depthCubeMapFbo;
    
    int numOmniShadowMaps;
};

#endif