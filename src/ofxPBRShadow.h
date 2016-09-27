#ifndef ofxPBRShadow_h
#define ofxPBRShadow_h

#include "ofMain.h"

class ofxPBRShadow {
public:
	void setup(int resolution);
	void resizeDepthMap(int resolution);
	void setNumLights(int numLights);
	void beginDepthMap(int index);
	void beginDepthMap();
	void endDepthMap();
	int getDepthMapResolution();

	void bind(GLuint index);
	void unbind();

private:
	void setShadowMap();

	int depthMapRes;
	int numLights;

	GLuint depthMapIndex, depthMapFbo;
};

#endif