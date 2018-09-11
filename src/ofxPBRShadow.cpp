#include "ofxPBRShadow.h"

void ofxPBRShadow::setup(int maxShadow, int resolution) {
	this->maxShadow = maxShadow;
	this->depthMapRes = resolution;
	viewProjMatrix.assign(maxShadow, glm::mat4());
	shadowMatrix.assign(maxShadow, glm::mat4());
	initFbo();
}

void ofxPBRShadow::resizeDepthMap(int resolution) {
	this->depthMapRes = resolution;
	initFbo();
}

void ofxPBRShadow::setMaxShadow(int maxShadow) {
	this->maxShadow = maxShadow;
	viewProjMatrix.assign(maxShadow, glm::mat4());
	shadowMatrix.assign(maxShadow, glm::mat4());
	initFbo();
}

void ofxPBRShadow::initFbo() {
	glGenTextures(1, &depthMapIndex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapIndex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapRes, depthMapRes, maxShadow, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	glGenFramebuffers(1, &depthMapFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapIndex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ofxPBRShadow::beginDepthMap(int index, ofCamera * depthCam) {
	viewProjMatrix[index] = depthCam->getModelViewProjectionMatrix(ofRectangle(0, 0, depthMapRes, depthMapRes));
	shadowMatrix[index] = biasMatrix * viewProjMatrix[index];
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapIndex, 0, index);
	ofPushView();
	ofViewport(0, 0, depthMapRes, depthMapRes, false);
	ofClear(0);
}

void ofxPBRShadow::endDepthMap() {
	ofPopView();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ofxPBRShadow::bind(GLuint index) {
	glActiveTexture(GL_TEXTURE0 + index);
	glEnable(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapIndex);
}

void ofxPBRShadow::unbind() {
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glDisable(GL_TEXTURE_2D_ARRAY);
	glActiveTexture(GL_TEXTURE0);
}