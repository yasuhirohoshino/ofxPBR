#pragma once
#include "ofMain.h"

class ofxPBROmniShadow
{
public:
	void setup(int maxShadow, int resolution);
	void resizeDepthMap(int resolution);
	void setMaxShadow(int maxShadow);
	void updateMatrix(int index, ofCamera * lightCam);
	void beginDepthMap(int index, int face);
	void endDepthMap();
	void bind(GLuint location);
	void unbind();

	int getMaxShadow() { return maxShadow; }
	int getDepthMapResolution() { return depthMapRes; }
	glm::mat4 getViewProjMatrix(int index, int face) { return pointLightViewProjMat[face][index]; }

private:
	void initFbo();

	int depthMapRes = 1024;
	int maxShadow = 0;
    GLuint location;
    GLuint texIndex;
    GLuint fbo;

	vector<glm::mat4> pointLightViewProjMat[6];
};