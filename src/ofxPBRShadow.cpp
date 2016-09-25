#include "ofxPBRShadow.h"

void ofxPBRShadow::setup(int resolution){
    blurShader.setupShaderFromSource(GL_VERTEX_SHADER, blur.gl3VertShader);
    blurShader.setupShaderFromSource(GL_FRAGMENT_SHADER, blur.gl3FragShader);
    blurShader.bindDefaults();
    blurShader.linkProgram();
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
//        settings.width  = depthMapRes;
//        settings.height = depthMapRes;
//        settings.textureTarget = GL_TEXTURE_2D;
//        settings.useDepth = false;
//        settings.depthStencilAsTexture = false;
//        settings.useStencil = false;
//        settings.minFilter = GL_LINEAR;
//        settings.maxFilter = GL_LINEAR;
//        settings.wrapModeHorizontal = GL_CLAMP_TO_EDGE;
//        settings.wrapModeVertical = GL_CLAMP_TO_EDGE;
//        
//        settings.internalformat = GL_R32F;
//        blurHFbo.allocate(settings);
//        blurVFbo.allocate(settings);
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

void ofxPBRShadow::setOmniShadowMap(int numOmniShadowMaps){
    depthCubeMapIndex.assign(numOmniShadowMaps, 0);
    depthCubeMapFbo.assign(numOmniShadowMaps, 0);
    for(int i=0;i<depthCubeMapIndex.size();i++){
        // generate cubemap fbo
        glGenFramebuffers(1, &depthCubeMapFbo[i]);
        glGenTextures(1, &depthCubeMapIndex[i]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMapIndex[i]);
        for (GLuint j = 0; j < 6; ++j){
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, 0, GL_DEPTH_COMPONENT32F, depthMapRes, depthMapRes, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        }
        
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFbo[i]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMapIndex[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void ofxPBRShadow::beginDepthMap(ofxPBRLight * pbrLight, int index){
    this->pbrLight = pbrLight;
	currentLightIndex = index;
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapIndex, 0, index);
    glViewport(0, 0, depthMapRes, depthMapRes);
    ofClear(0);
}

void ofxPBRShadow::endDepthMap(){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
//    if(pbrLight->getShadowType() == ShadowType_Soft){
//        blurVFbo.begin();
//        ofClear(255, 0, 0, 255);
//        bind(1);
//        blurShader.begin();
//        blurShader.setUniform1i("index", currentLightIndex);
//        blurShader.setUniform1i("isFirstPass", true);
//        blurShader.setUniform2f("resolution", blurHFbo.getWidth(), blurHFbo.getHeight());
//        blurShader.setUniform1f("sigma", 2.0);
//        blurShader.setUniform1i("rawSampler", 1);
//        blurShader.setUniform1i("horizontal", 0);
//        ofDrawPlane(0, 0, blurHFbo.getWidth(), blurHFbo.getHeight());
//        blurShader.end();
//        blurVFbo.end();
//        unbind();
//        
//        blurHFbo.begin();
//        ofClear(255, 0, 0, 255);
//        blurShader.begin();
//        blurShader.setUniform1i("isFirstPass", false);
//        blurShader.setUniform2f("resolution", blurHFbo.getWidth(), blurHFbo.getHeight());
//        blurShader.setUniform1f("sigma", 2.0);
//        blurShader.setUniformTexture("blurSampler", blurVFbo.getTexture(), 1);
//        blurShader.setUniform1i("horizontal", 1);
//        blurVFbo.draw(0, 0);
//        blurShader.end();
//        blurHFbo.end();
//        
//        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
//        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapIndex, 0, currentLightIndex);
//        glViewport(0, 0, depthMapRes, depthMapRes);
//        ofClear(0);
//        blurHFbo.draw(0, 0);
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//    }
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