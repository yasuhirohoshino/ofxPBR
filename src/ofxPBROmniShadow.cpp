#include "ofxPBROmniShadow.h"

void ofxPBROmniShadow::setup(int maxShadow, int resolution)
{
	this->maxShadow = maxShadow;
	this->depthMapRes = resolution;
	for (int i = 0; i < 6; i++) {
		pointLightViewProjMat[i].assign(maxShadow, glm::mat4());
	}
	initFbo();
}

void ofxPBROmniShadow::resizeDepthMap(int resolution)
{
	this->depthMapRes = resolution;
	initFbo();
}

void ofxPBROmniShadow::setMaxShadow(int maxShadow)
{
	this->maxShadow = maxShadow;
	pointLightViewProjMat->resize(maxShadow);
	initFbo();
}

void ofxPBROmniShadow::updateMatrix(int index, ofCamera * lightCam)
{
	glm::mat4 shadowProjMatrix;
	shadowProjMatrix = glm::perspective(ofDegToRad(90), 1.0f, 1.0f, lightCam->getFarClip());

	glm::mat4 pointLightLookAtMat[6];
	pointLightLookAtMat[0] = (glm::lookAt(lightCam->getGlobalPosition(), lightCam->getGlobalPosition() + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)));
	pointLightLookAtMat[1] = (glm::lookAt(lightCam->getGlobalPosition(), lightCam->getGlobalPosition() + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)));
	pointLightLookAtMat[2] = (glm::lookAt(lightCam->getGlobalPosition(), lightCam->getGlobalPosition() + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)));
	pointLightLookAtMat[3] = (glm::lookAt(lightCam->getGlobalPosition(), lightCam->getGlobalPosition() + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)));
	pointLightLookAtMat[4] = (glm::lookAt(lightCam->getGlobalPosition(), lightCam->getGlobalPosition() + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)));
	pointLightLookAtMat[5] = (glm::lookAt(lightCam->getGlobalPosition(), lightCam->getGlobalPosition() + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0)));

	// make view projection matricies
	for (int i = 0; i < 6; i++) {
		pointLightViewProjMat[i][index] = shadowProjMatrix * pointLightLookAtMat[i];
	}
}

void ofxPBROmniShadow::beginDepthMap(int index, int face)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texIndex, 0, (index * 6) + face);
	glViewport(0, 0, depthMapRes, depthMapRes);
	ofClear(0);
    glDrawBuffer(GL_NONE);
}

void ofxPBROmniShadow::endDepthMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ofxPBROmniShadow::bind(GLuint location)
{
    this->location = location;
	glActiveTexture(GL_TEXTURE0 + location);
	glEnable(GL_TEXTURE_CUBE_MAP_ARRAY);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texIndex);
}

void ofxPBROmniShadow::unbind()
{
	glActiveTexture(GL_TEXTURE0 + location);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);
	glDisable(GL_TEXTURE_CUBE_MAP_ARRAY);
	glActiveTexture(GL_TEXTURE0);
}

void ofxPBROmniShadow::initFbo()
{
    // generate cubemap fbo
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &texIndex);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, texIndex);

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapRes, depthMapRes, maxShadow * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texIndex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
