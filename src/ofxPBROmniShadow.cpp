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

int ofxPBROmniShadow::getDepthMapResolution()
{
	return depthMapRes;
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

void ofxPBROmniShadow::setOmniShadowMap(int numOmniShadowMaps)
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
    glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapRes, depthMapRes, (numOmniShadowMaps + 1) * 6, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texIndex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
