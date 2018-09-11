#pragma once
#include "ofMain.h"
#include "ofxPBRCubeMap.h"
#include "ofxPBRLight.h"
#include "ofxPBRSpotShadow.h"
#include "ofxPBROmniShadow.h"
#include "ofxPBRDirectionalShadow.h"
#include "ofxPBRMaterial.h"
#include "shaders/environment.h"
#include "shaders/pbr.h"

enum RenderMode {
	Mode_PBR = 0,
	Mode_DirectionalShadow = 1,
	Mode_SpotShadow = 2,
	Mode_OmniShadow = 3,
	num_Mode = 4
};

class ofxPBR{
public:
    ofxPBR();
    
	void setup(function<void()> scene, ofCamera* camera, int depthMapResolution = 1024);
	void updateDepthMaps();
	void renderScene();

	// renderer
	void beginDefaultRenderer();
	void endDefaultRenderer();
	void beginCustomRenderer(ofShader* shader = nullptr);
	void endCustomRenderer();

	// cube map
	void setCubeMap(ofxPBRCubeMap* cubeMap);
	void enableCubeMap(bool enable);
    
	// environment
	void setDrawEnvironment(bool enable);
	void drawEnvironment();
	void setEnvShader(ofShader* shader);

	// shadow
	void setMaxShadow(int maxShadow);
	void setMaxOmniShadow(int maxOmniShadow);
	void setMaxDirectionalShadow(int maxDirectionalShadow);
	void resizeDepthMap(int resolution);
	void setDirectionalShadowBBox(float x, float y, float z, float width, float height, float depth);
	void setUsingCameraFrustomForShadow(bool usingCameraFrustom);
	void setCameraForDirectionalShadowBBox(ofCamera* camera);

	// light
	void addLight(ofxPBRLight* light);
	void removeLight(int index);
    
	// getter
	RenderMode getRenderMode() { return renderMode; }
	bool isCubeMapEnabled() { return enableCubemap; }

	int getMaxShadow() { return spotShadow.getMaxShadow(); }
	int getMaxOmniShadow() { return omniShadow.getMaxShadow(); }
	int getMaxDirectionalShadow() { return directionalShadow.getMaxShadow(); }

	int getDepthMapResolution() { return spotShadow.getDepthMapResolution(); }

	ofShader* getShader() { return PBRShader; }
	int getLastTextureIndex() { return 12; }

private:
	void begin();
	void end();
    
    void beginPBR();
    void endPBR();
    
    void beginSpotDepthMap();
	void beginDepthCubeMap();
	void beginDirectionalDepthMap();

    ofMesh sphereMesh;
	bool enableDrawEnvironment = false;
    
	ofxPBRCubeMap* cubeMap;
	bool enableCubemap = false;

	ofCamera * camera;

	RenderMode renderMode;

	vector<ofxPBRLight*> lights;
	int currentLightIndex = 0;

	// shaders
	ofShader* PBRShader;
    ofShader* envShader;
	ofShader defaultShader;

	PBR pbrShaderSource;
    Environment environmentShaderSource;
    
	// shadow
	ofxPBRSpotShadow spotShadow;
	ofxPBROmniShadow omniShadow;
	ofxPBRDirectionalShadow directionalShadow;

    int spotShadowIndex = 0;
	int omniShadowIndex = 0;
	int omniShadowFace = 0;
	int directionalShadowIndex = 0;

	function<void()> scene;
};