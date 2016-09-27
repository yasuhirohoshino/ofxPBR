#include "ofxPBROmniShadow.h"

void ofxPBROmniShadow::setup(int resolution)
{
	depthMapRes = resolution;
}

void ofxPBROmniShadow::resizeDepthMap(int resolution)
{
	depthMapRes = resolution;
	setOmniShadowMap(numOmniShadowMaps);
}

void ofxPBROmniShadow::setNumLights(int numLights)
{
	numOmniShadowMaps = numLights;
	setOmniShadowMap(numLights);
}

void ofxPBROmniShadow::beginDepthMap(int index, int face)
{
	glBindFramebuffer(GL_FRAMEBUFFER, shadowParams[index].fbo);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowParams[index].index, 0, face);
	glViewport(0, 0, depthMapRes, depthMapRes);
	ofClear(0);
}

void ofxPBROmniShadow::endDepthMap()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int ofxPBROmniShadow::getDepthMapResolution()
{
	return depthMapRes;
}

void ofxPBROmniShadow::bind(int index, GLuint location)
{
	glActiveTexture(GL_TEXTURE0 + location);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, shadowParams[index].index);
}

void ofxPBROmniShadow::unbind(GLuint location)
{
	glActiveTexture(GL_TEXTURE0 + location);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glActiveTexture(GL_TEXTURE0);
}

void ofxPBROmniShadow::setOmniShadowMap(int numOmniShadowMaps)
{
	shadowParams.assign(numOmniShadowMaps, OmniShadowParams());
	for (int i = 0; i < shadowParams.size(); i++) {
		// generate cubemap fbo
		glGenFramebuffers(1, &shadowParams[i].fbo);
		glGenTextures(1, &shadowParams[i].index);
		glBindTexture(GL_TEXTURE_CUBE_MAP, shadowParams[i].index);
		for (GLuint j = 0; j < 6; ++j) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT32F, depthMapRes, depthMapRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, shadowParams[i].fbo);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowParams[i].index, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}
