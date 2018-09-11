#pragma once
#include "ofMain.h"

class ofxPBRShadow {
public:
	void setup(int maxShadow, int resolution);
	void resizeDepthMap(int resolution);
	void setMaxShadow(int maxShadow);
	void beginDepthMap(int index, ofCamera * depthCam);
	void endDepthMap();
	void bind(GLuint index);
	void unbind();

	int getMaxShadow() { return maxShadow; }
	int getDepthMapResolution() { return depthMapRes; }
	glm::mat4 getViewProjMatrix(int index) { return viewProjMatrix[index]; }
	vector<glm::mat4> getShadowMatrix() { return shadowMatrix; }

protected:
	void initFbo();

	int depthMapRes;
	int maxShadow;
	GLuint depthMapIndex;
	GLuint depthMapFbo;
	int currentTexIndex = 0;

	vector<glm::mat4> viewProjMatrix;
	vector<glm::mat4> shadowMatrix;
	const glm::mat4 biasMatrix = glm::mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);
};