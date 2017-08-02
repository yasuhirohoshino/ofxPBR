#pragma once
#include "ofMain.h"
#include "ofxPBRCubeMap.h"
#include "ofxPBRLight.h"
#include "ofxPBRSpotShadow.h"
#include "ofxPBROmniShadow.h"
#include "ofxPBRCascadeShadow.h"
#include "ofxPBRDirectionalShadow.h"
#include "ofxPBRMaterial.h"
#include "shaders/environment.h"
#include "shaders/pbr.h"
#include "shaders/depthThumbnail.h"

enum RenderMode {
	Mode_PBR = 0,
	Mode_SpotShadow = 1,
	Mode_OmniShadow = 2,
	Mode_CascadeShadow = 3,
	Mode_DirectionalShadow = 4,
	num_Mode = 5
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

	// camera
	void setCamera(ofCamera* camera);

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
	void setMaxCascadeShadow(int maxCascadeShadow);
	void setMaxDirectionalShadow(int maxDirectionalShadow);
	void resizeDepthMap(int resolution);
	void setDirectionalShadowBB(float x, float y, float z, float width, float height, float depth);
	void setUsingCameraFrustom(bool usingCameraFrustom);

	// light
	void addLight(ofxPBRLight* light);
	void removeLight(int index);
    
	// getter
	RenderMode getRenderMode() { return renderMode; }
	bool isCubeMapEnabled() { return enableCubemap; }

	int getMaxShadow() { return spotShadow.getMaxShadow(); }
	int getMaxOmniShadow() { return omniShadow.getMaxShadow(); }
	int getMaxCascadeShadow() { return 0; }
	int getNumCascade() { return numCascade; }
	int getMaxDirectionalShadow() { return directionalShadow.getMaxShadow(); }

	int getDepthMapResolution() { return spotShadow.getDepthMapResolution(); }
	ofTexture* getDepthMap(int index);

	ofShader* getShader() { return PBRShader; }
	int getLastTextureIndex() { return 12; }

private:
	void begin();
	void end();
    
    void beginPBR();
    void endPBR();
    
    void beginSpotDepthMap();
	void beginDepthCubeMap();
	void beginCascadeDepthMap();
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
	ofxPBRCascadeShadow cascadeShadow;
	ofxPBRDirectionalShadow directionalShadow;

    int spotShadowIndex = 0;
	int omniShadowIndex = 0;
	int omniShadowFace = 0;
	int directionalShadowIndex = 0;

	int cascadeShadowIndex = 0;
	int currentCascade = 0;
	vector<float> cascadeDistances;
	int numCascade = 3;
    
	// depth map thumbnail
    DepthThumbnail depthThumbnail;
    ofFbo depthThumbnailFbo;
    ofShader depthThumbnailShader;

	function<void()> scene;
};