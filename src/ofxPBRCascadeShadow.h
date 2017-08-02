#pragma once
#include "ofMain.h"

class ofxPBRCascadeShadow
{
public:
	void setup(int maxShadow, vector<float> clipDistance, int resolution);
	void resizeDepthMap(int resolution);
	void setMaxShadow(int maxShadow);
	void setNumCascade(int numCascade);
	void updateCameraFrustom(ofCamera* cam);
	void updateLightMatrix(int index, ofCamera* light);
	void beginDepthMap(int index, int cascade);
	void endDepthMap();
	void bind(GLuint location);
	void unbind();

	int getMaxShadow() { return maxShadow; }
	int getNumCascade() { return numCascade; }
	int getDepthMapResolution() { return depthMapRes; }
	ofMatrix4x4 getViewProjMatrix(int index, int cascade) { return cascadeShadowData[index][cascade].depthCamera.getModelViewProjectionMatrix(ofRectangle(0, 0, depthMapRes, depthMapRes)); }
	vector<ofMatrix4x4> getShadowMatrix() { return shadowMatrix; }
	vector<float> getClips() { return clips; }

private:
	void initFbo();

	int depthMapRes = 1024;
	int numCascade = 3;
	int maxShadow = 0;
	GLuint location;
	GLuint texIndex;
	GLuint fbo;
	vector<float> clips;
	vector<float> clipDistances;

	struct FrustomData {
		float nearClip;
		float farClip;
		ofVec3f corners[8];
	};
	vector<FrustomData> frustomData;

	struct CascadeShadowData {
		ofMatrix4x4 shadowMatrix;
		ofCamera depthCamera;
	};
	vector<vector<CascadeShadowData>> cascadeShadowData;

	ofMatrix4x4 cameraViewProjectionMatrix;
	ofMatrix4x4 cameraInverseViewMmatrix;

	vector<ofMatrix4x4> shadowMatrix;
	const ofMatrix4x4 biasMatrix = ofMatrix4x4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
};