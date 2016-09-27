#include "ofxPBRShadow.h"

void ofxPBRShadow::setup(int resolution){
    depthMapRes = resolution;
}

void ofxPBRShadow::resizeDepthMap(int resolution){
    depthMapRes = resolution;
    setShadowMap();
}

void ofxPBRShadow::setNumLights(int numLights){
    this -> numLights = numLights;
    setShadowMap();
}

void ofxPBRShadow::setShadowMap(){
    glGenTextures(1, &depthMapIndex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapIndex);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapRes, depthMapRes, numLights, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    
    glGenFramebuffers(1, &depthMapFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapIndex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ofxPBRShadow::beginDepthMap(int index){    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapIndex, 0, index);
    glViewport(0, 0, depthMapRes, depthMapRes);
    ofClear(0);
}

void ofxPBRShadow::beginDepthMap() {
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
	glViewport(0, 0, depthMapRes, depthMapRes);
	ofClear(0);
}

void ofxPBRShadow::endDepthMap(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int ofxPBRShadow::getDepthMapResolution(){
    return depthMapRes;
}

void ofxPBRShadow::bind(GLuint index){
    glActiveTexture(GL_TEXTURE0 + index);
    glEnable(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapIndex);
}

void ofxPBRShadow::unbind(){
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0 );
    glDisable( GL_TEXTURE_2D_ARRAY );
    glActiveTexture( GL_TEXTURE0 );
}