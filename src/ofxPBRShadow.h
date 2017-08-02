#pragma once
#include "ofMain.h"

class ofxPBRShadow {
public:
	void setup(int maxShadow, int resolution);
	void resizeDepthMap(int resolution);
	void setMaxShadow(int maxShadow);
	void beginDepthMap(int index, ofCamera * cam, ofCamera * depthCam);
	void endDepthMap();
	void bind(GLuint index);
	void unbind();

	int getMaxShadow() { return maxShadow; }
	int getDepthMapResolution() { return depthMapRes; }
	ofMatrix4x4 getViewProjMatrix(int index) { return viewProjMatrix[index]; }
	vector<ofMatrix4x4> getShadowMatrix() { return shadowMatrix; }

protected:
	void initFbo();

	int depthMapRes;
	int maxShadow;
	GLuint depthMapIndex;
	GLuint depthMapFbo;
	int currentTexIndex = 0;

	vector<ofMatrix4x4> viewProjMatrix;
	vector<ofMatrix4x4> shadowMatrix;
	const ofMatrix4x4 biasMatrix = ofMatrix4x4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
};