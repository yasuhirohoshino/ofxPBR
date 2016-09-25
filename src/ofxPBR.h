#pragma once
#include "ofMain.h"
#include "ofxPBRCubeMap.h"
#include "ofxPBRLight.h"
#include "ofxPBRShadow.h"
#include "ofxPBRMaterial.h"
#include "shaders/environment.h"
#include "shaders/pbr.h"

class ofxPBR{
public:
    ofxPBR();
    
	void setup(int depthMapResolution);
    void begin(ofCamera * camera, ofShader * shader = nullptr);
    void end();
    
	void setCubeMap(ofxPBRCubeMap * cubeMap);
	void enableCubeMap(bool enable);
	bool isCubeMapEnable();
    void drawEnvironment(ofCamera * camera);
    void resizeDepthMap(int resolution);
    int getDepthMapResolution();
	void makeDepthMap(function<void()> scene);
	void addLight(ofxPBRLight* light);
	void removeLight(int index);
    void setEnvShader(ofShader* shader);
    ofShader* getShader();
    
private:
	void setNumLights(int numLights);
    
    void beginPBR(ofCamera * camera);
    void endPBR();
    
    void beginDepthMap();
    void endDepthMap();

    ofxPBRShadow shadow;
    ofShader * PBRShader;
    ofMesh sphereMesh;
    ofxPBRCubeMap * cubeMap;
    vector<ofMatrix4x4> shadowMatrix;
	bool depthMapMode;
	vector<ofxPBRLight *> lights;
	bool enableCubemap;
    
    ofShader* envShader;
    Environment environment;
    ofShader defaultShader;
    PBR pbr;
    
    int lightIndex;
};