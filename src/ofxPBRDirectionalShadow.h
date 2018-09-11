#pragma once
#include "ofMain.h"
#include "ofxPBRShadow.h"

class ofxPBRDirectionalShadow: public ofxPBRShadow {
public:
	void setup(int maxShadow, int resolution);
	void calcCorners(ofCamera * cam);
	void setBoundingBox(float x, float y, float z, float width, float height, float depth);
	void setUsingCameraFrustom(bool usingCameraFrustom);
	void beginDepthMap(int index, ofCamera * light);

	bool isUsingCameraFrustom() { return usingCameraFrustom; }

private:
	ofVec3f corners[8];
	ofVec3f BBCenter;
	float BBWidth, BBHeight, BBDepth;
	float negX, posX, negY, posY, negZ, posZ;
	ofCamera depthCam;
	glm::mat4 cameraInverseViewMmatrix;
	bool usingCameraFrustom = true;
};