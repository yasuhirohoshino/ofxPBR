#pragma once
#include "ofMain.h"

class ofxPBROmniShadow
{
public:
	void setup(int resolution);
	void resizeDepthMap(int resolution);
	void setNumLights(int numLights);
	void beginDepthMap(int index, int face);
	void endDepthMap();
	int getDepthMapResolution();

	void bind(GLuint location);
	void unbind();

private:
	void setOmniShadowMap(int numOmniShadowMaps);
	int depthMapRes = 1024;
	int numOmniShadowMaps = 0;
    GLuint location;
    GLuint texIndex;
    GLuint fbo;
};