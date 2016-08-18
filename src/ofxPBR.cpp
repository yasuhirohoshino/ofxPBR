#include "ofxPBR.h"

ofxPBR::ofxPBR(){
}

void ofxPBR::setup(int depthMapResolution) {
	sphereMesh = ofSpherePrimitive(1, 100).getMesh();
	for (int i = 0; i<sphereMesh.getNormals().size(); i++) {
		sphereMesh.setNormal(i, ofVec3f(1.0, 1.0, -1.0) * sphereMesh.getVertex(i).normalize());
	}
	shadow.setup(depthMapResolution);
	depthMapMode = false;
	enableCubemap = false;
    
    envShader = new ofShader();
    envShader->setupShaderFromSource(GL_VERTEX_SHADER, environment.gl3VertShader);
    envShader->setupShaderFromSource(GL_FRAGMENT_SHADER, environment.gl3FragShader);
    envShader->bindDefaults();
    envShader->linkProgram();
    
    defaultShader = ofShader();
    defaultShader.setupShaderFromSource(GL_VERTEX_SHADER, pbr.gl3VertShader);
    defaultShader.setupShaderFromSource(GL_FRAGMENT_SHADER, pbr.gl3FragShader);
    defaultShader.bindDefaults();
    defaultShader.linkProgram();
}

void ofxPBR::begin(ofCamera * camera, ofShader * shader){
    if(shader != nullptr){
        PBRShader = shader;
    }else{
        PBRShader = &defaultShader;
    }
	if (depthMapMode) {
		PBRShader->begin();
		PBRShader->setUniform1i("renderForDepthMap", true);
	}else{
		PBRShader->begin();
		PBRShader->setUniform1i("renderForDepthMap", false);
		PBRShader->setUniform1f("cameraNear", camera->getNearClip());
		PBRShader->setUniform1f("cameraFar", camera->getFarClip());
		if (enableCubemap && cubeMap != nullptr && cubeMap->isAllocated()) {
            cubeMap->bind(1);
            PBRShader->setUniform1i("enableEnv", true);
			PBRShader->setUniform1i("numMips", cubeMap->getNumMips());
			PBRShader->setUniform1f("cubeMapExposure", cubeMap->getExposure());
			PBRShader->setUniform1f("cubeMapRotation", cubeMap->getRotation());
			PBRShader->setUniform1i("isHDR", cubeMap->isHDR());
		}else{
            PBRShader->setUniform1i("enableEnv", false);
            PBRShader->setUniform1i("isHDR", false);
		}
        PBRShader->setUniform1i("envMap", 1);
		PBRShader->setUniformMatrix4f("viewTranspose", ofMatrix4x4::getTransposedOf(camera->getModelViewMatrix()));
		PBRShader->setUniformMatrix4f("viewMatrix", ofGetCurrentViewMatrix());
		PBRShader->setUniform1i("numLights", lights.size());
		for (int i = 0; i<lights.size(); i++) {
            if (cubeMap != nullptr && lights[i]->getLightType() == LightType_Sky && lights[i]->getShadowType() != ShadowType_None){
                lights[i]->setSkyLightRotation(cubeMap->getRotation());
            }
			shadowMatrix[i] = lights[i]->getShadowMatrix(camera->getModelViewMatrix());
			lights[i]->beginLighting(PBRShader, i);
		}
		if (lights.size() != 0) {
			PBRShader->setUniformTexture("shadowMap", *shadow.getDepthMap(), 10);
			PBRShader->setUniform2f("depthMapAtrasRes", shadow.getDepthMapAtrasRes());
			PBRShader->setUniform2f("depthTexMag", shadow.getDepthTexMag());
			glUniformMatrix4fv(PBRShader->getUniformLocation("shadowMatrix"), lights.size(), false, shadowMatrix[0].getPtr());
		}
	}
}

void ofxPBR::end(){
	for (int i = 0; i<lights.size(); i++) {
		lights[i]->endLighting(PBRShader);
	}
    if (enableCubemap && cubeMap !=nullptr && cubeMap->isAllocated()) {
        cubeMap->unbind();
    }
    PBRShader->end();
}

void ofxPBR::setCubeMap(ofxPBRCubeMap * cubeMap)
{
	this->cubeMap = cubeMap;
	enableCubemap = true;
}

void ofxPBR::enableCubeMap(bool enable)
{
	enableCubemap = enable;
}

bool ofxPBR::isCubeMapEnable()
{
	return enableCubemap;
}

void ofxPBR::drawEnvironment(ofCamera * camera){
    if (enableCubemap && cubeMap != nullptr && cubeMap->isAllocated()) {
        float scale = (camera->getFarClip() - camera->getNearClip()) / 2;
        ofDisableDepthTest();
        ofPushMatrix();
        ofTranslate(camera->getPosition());
        cubeMap->bind(1);
        envShader->begin();
        envShader->setUniform1f("envLevel", cubeMap->getEnvLevel());
        envShader->setUniform1i("envMap", 1);
        envShader->setUniform1i("numMips", cubeMap->getNumMips());
        envShader->setUniform1f("cubeMapExposure", cubeMap->getExposure());
        envShader->setUniform1f("cubeMapRotation", cubeMap->getRotation());
        ofPushMatrix();
        ofScale(scale, scale, scale);
        sphereMesh.draw();
        ofPopMatrix();
        envShader->end();
        cubeMap->unbind();
        ofPopMatrix();
        ofEnableDepthTest();
    }
}

void ofxPBR::resizeDepthMap(int resolution){
    shadow.resizeDepthMap(resolution);
	for(auto light: lights) {
		light->setDepthMapRes(resolution);
	}
}

int ofxPBR::getDepthMapResolution(){
    return shadow.getDepthMapResolution();
}

ofTexture * ofxPBR::getDepthMap(){
    return shadow.getDepthMap();
}

void ofxPBR::makeDepthMap(function<void()> scene)
{
	for (int i = 0; i<lights.size(); i++) {
		if (lights[i]->getShadowType() != ShadowType_None && lights[i]->isEnabled()) {
			if (lights[i]->getLightType() == LightType_Sky && cubeMap != nullptr) lights[i]->setSkyLightRotation(cubeMap->getRotation());
			depthMapMode = true;
			shadow.beginDepthMap(lights[i], i);
			scene();
			shadow.endDepthMap();
			depthMapMode = false;
		}
	}
}

void ofxPBR::addLight(ofxPBRLight * light) {
	for (auto l : lights) {
		if (l == light) return;
	}
	lights.push_back(light);
	light->setDepthMapRes(getDepthMapResolution());
	setNumLights(lights.size());
}

void ofxPBR::removeLight(int index) {
	lights.erase(lights.begin() + index);
	setNumLights(lights.size());
}

void ofxPBR::setEnvShader(ofShader* shader){
    envShader = shader;
}

ofShader* ofxPBR::getShader(){
    return PBRShader;
}

int getLastTextureIndex(){
    return 11;
}

// private

void ofxPBR::setNumLights(int numLights) {
	shadow.setNumLights(numLights);
	shadowMatrix.assign(numLights, ofMatrix4x4());
	for (int i = 0; i < lights.size(); i++) {
		lights[i]->setId(i);
	}
}