#include "ofxPBRShadow.h"

void ofxPBRShadow::setup(int resolution){
//    blurShader.setupShaderFromSource(GL_VERTEX_SHADER, blur.gl3VertShader);
//    blurShader.setupShaderFromSource(GL_FRAGMENT_SHADER, blur.gl3FragShader);
//    blurShader.bindDefaults();
//    blurShader.linkProgram();
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
//    if(depthMapRes != 0 && numLights != 0){
//        depthMapAtrasWidth = depthMapRes * min(numLights, 4);
//        depthMapAtrasHeight = depthMapRes * (floor(numLights / 5) + 1);
//        
//        depthTexMag.x = float(depthMapRes) / float(depthMapAtrasWidth);
//        depthTexMag.y = float(depthMapRes) / float(depthMapAtrasHeight);
//        
//        settings.width  = depthMapRes;
//        settings.height = depthMapRes;
//        settings.textureTarget = GL_TEXTURE_2D;
//        settings.internalformat = GL_R8;
//        settings.useDepth = true;
//        settings.depthStencilAsTexture = true;
//        settings.useStencil = true;
//        settings.minFilter = GL_LINEAR;
//        settings.maxFilter = GL_LINEAR;
//        settings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
//        settings.wrapModeVertical = GL_CLAMP_TO_EDGE;
//        
//        depthMap.allocate(settings);
//        
//        settings.internalformat = GL_R32F;
//        blurHFbo.allocate(settings);
//        blurVFbo.allocate(settings);
//        
//        settings.width  = depthMapAtrasWidth;
//        settings.height = depthMapAtrasHeight;
//        
//        depthSumFbo.allocate(settings);
//        depthSumFbo.begin();
//        ofClear(255, 0, 0);
//        depthSumFbo.end();
//    }
    
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

void ofxPBRShadow::beginDepthMap(ofxPBRLight * pbrLight, int index){
    this->pbrLight = pbrLight;
	currentLightIndex = index;
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapIndex, 0, index);
    glViewport(0, 0, depthMapRes, depthMapRes);
    ofClear(0);
}

void ofxPBRShadow::beginDepthMap(){
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glViewport(0, 0, depthMapRes, depthMapRes);
    ofClear(0);
}

void ofxPBRShadow::endDepthMap(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
//    if(pbrLight->getShadowType() == ShadowType_Soft){
//        blurVFbo.begin();
//        ofClear(255, 0, 0, 255);
//        blurShader.begin();
//        blurShader.setUniform2f("resolution", blurHFbo.getWidth(), blurHFbo.getHeight());
//        blurShader.setUniform1f("sigma", 2.0);
//        blurShader.setUniformTexture("blurSampler", depthMap.getDepthTexture(), 1);
//        blurShader.setUniform1i("horizontal", 0);
//        depthMap.getDepthTexture().draw(0, 0);
//        blurShader.end();
//        blurVFbo.end();
//        
//        blurHFbo.begin();
//        ofClear(255, 0, 0, 255);
//        blurShader.begin();
//        blurShader.setUniform2f("resolution", blurHFbo.getWidth(), blurHFbo.getHeight());
//        blurShader.setUniform1f("sigma", 2.0);
//        blurShader.setUniformTexture("blurSampler", blurVFbo.getTexture(), 1);
//        blurShader.setUniform1i("horizontal", 1);
//        blurVFbo.draw(0, 0);
//        blurShader.end();
//        blurHFbo.end();
//    }
    
//	int index = currentLightIndex;
//    depthSumFbo.begin();
//    if(pbrLight->getShadowType() == ShadowType_Hard){
//        depthMap.getDepthTexture().draw((index % 4) * depthMapRes, floor(float(index) / 4) * depthMapRes);
//    }else if(pbrLight->getShadowType() == ShadowType_Soft){
//        blurHFbo.draw((index % 4) * depthMapRes, floor(float(index) / 4) * depthMapRes);
//    }
//    depthSumFbo.end();
}

ofTexture * ofxPBRShadow::getDepthMap(){
    return &depthSumFbo.getTexture();
}

int ofxPBRShadow::getDepthMapResolution(){
    return depthMapRes;
}

ofVec2f ofxPBRShadow::getDepthMapAtrasRes(){
    return ofVec2f(depthMapAtrasWidth, depthMapAtrasHeight);
}

ofVec2f ofxPBRShadow::getDepthTexMag(){
    return depthTexMag;
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