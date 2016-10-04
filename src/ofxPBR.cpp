#include "ofxPBR.h"

ofxPBR::ofxPBR(){
}

void ofxPBR::setup(ofCamera* camera, function<void()> scene, int depthMapResolution) {
	this->camera = camera;
	this->scene = scene;
	sphereMesh = ofSpherePrimitive(1, 100).getMesh();
	for (int i = 0; i<sphereMesh.getNormals().size(); i++) {
		sphereMesh.setNormal(i, ofVec3f(1.0, 1.0, -1.0) * sphereMesh.getVertex(i).normalize());
	}
	shadow.setup(depthMapResolution);
	omniShadow.setup(depthMapResolution);
	renderMode = Mode_PBR;
	enableCubemap = false;
    
    // load env shader
    envShader = new ofShader();
    envShader->setupShaderFromSource(GL_VERTEX_SHADER, environment.gl3VertShader);
    envShader->setupShaderFromSource(GL_FRAGMENT_SHADER, environment.gl3FragShader);
    envShader->bindDefaults();
    envShader->linkProgram();
    
    // load defalut pbr shader
    defaultShader = ofShader();
    defaultShader.setupShaderFromSource(GL_VERTEX_SHADER, pbr.gl3VertShader);
    defaultShader.setupShaderFromSource(GL_FRAGMENT_SHADER, pbr.gl3FragShader);
    defaultShader.bindDefaults();
    defaultShader.linkProgram();
    
    depthThumbnailFbo.allocate(depthMapResolution, depthMapResolution, GL_R32F);
    
    depthThumbnailShader = ofShader();
    depthThumbnailShader.setupShaderFromSource(GL_VERTEX_SHADER, depthThumbnail.gl3VertShader);
    depthThumbnailShader.setupShaderFromSource(GL_FRAGMENT_SHADER, depthThumbnail.gl3FragShader);
    depthThumbnailShader.bindDefaults();
    depthThumbnailShader.linkProgram();
}

void ofxPBR::begin(ofShader * shader, ofCamera * camera){
	ofCamera* cam;
	if (camera != nullptr) {
		cam = camera;
	}else{
		cam = this->camera;
	}
    if(shader != nullptr){
        PBRShader = shader;
    }else{
        PBRShader = &defaultShader;
    }

	switch (renderMode)
	{
	case Mode_PBR:
		beginPBR(cam);
		break;

	case Mode_Shadow:
		beginDepthMap(lightIndex);
		break;

	case Mode_OmniShadow:
		beginDepthCubeMap(pointLightIndex, faceIndex);
		break;

	default:
		break;
	}
}

void ofxPBR::end(){
	switch (renderMode)
	{
	case Mode_PBR:
		endPBR();
		break;

	case Mode_Shadow:
		endDepthMap();
		break;

	case Mode_OmniShadow:
		endDepthCubeMap();
		break;

	default:
		break;
	}
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
	ofCamera* cam;
	if (camera != nullptr) {
		cam = camera;
	}
	else {
		cam = this->camera;
	}
    if (enableCubemap && cubeMap != nullptr && cubeMap->isAllocated()) {
        float scale = (cam->getFarClip() - cam->getNearClip()) / 2;
        ofDisableDepthTest();
        ofPushMatrix();
        ofTranslate(cam->getPosition());
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

void ofxPBR::updateDepthMaps()
{
	renderMode = Mode_Shadow;
	for (int i = 0; i<normalLights.size(); i++) {
		if (normalLights[i]->getShadowType() != ShadowType_None && normalLights[i]->isEnabled()) {
            if (normalLights[i]->getLightType() == LightType_Sky && cubeMap != nullptr){
				normalLights[i]->setSkyLightRotation(cubeMap->getRotation());
            }
            lightIndex = i;
			shadow.beginDepthMap(i);
			scene();
			shadow.endDepthMap();
		}
	}

	renderMode = Mode_OmniShadow;
	for (int i = 0; i < pointLights.size(); i++) {
		if (pointLights[i]->getShadowType() != ShadowType_None && pointLights[i]->isEnabled()) {
			pointLights[i]->updateOmniShadowParams();
			pointLightIndex = i;
			for (int j = 0; j < 6; j++) {
				faceIndex = j;
				omniShadow.beginDepthMap(i, j);
				scene();
				omniShadow.endDepthMap();
			}
		}
	}

	renderMode = Mode_PBR;
}

void ofxPBR::addLight(ofxPBRLight * light) {
	for (auto l : lights) {
		if (l == light) return;
	}
	lights.push_back(light);
	light->setDepthMapRes(getDepthMapResolution());
    light->setLightFunction(bind(&ofxPBR::setLights, this));
	setLights();
}

void ofxPBR::removeLight(int index) {
	lights.erase(lights.begin() + index);
	setLights();
}

void ofxPBR::setEnvShader(ofShader* shader){
    envShader = shader;
}

ofShader* ofxPBR::getShader(){
    return PBRShader;
}

int getLastTextureIndex(){
    return 12;
}

// private

void ofxPBR::beginPBR(ofCamera * camera){
    // pbr
    PBRShader->begin();
    PBRShader->setUniform1i("renderDepthMap", false);
    PBRShader->setUniform1f("cameraNear", camera->getNearClip());
    PBRShader->setUniform1f("cameraFar", camera->getFarClip());
    
    // cube map settings
    if (enableCubemap && cubeMap != nullptr && cubeMap->isAllocated()) {
        // enable cubemap
        cubeMap->bind(1);
        PBRShader->setUniform1i("enableEnv", true);
        PBRShader->setUniform1i("numMips", cubeMap->getNumMips());
        PBRShader->setUniform1f("cubeMapExposure", cubeMap->getExposure());
        PBRShader->setUniform1f("cubeMapRotation", cubeMap->getRotation());
        PBRShader->setUniform1i("isHDR", cubeMap->isHDR());
    }else{
        // disable cubemap
        PBRShader->setUniform1i("enableEnv", false);
        PBRShader->setUniform1i("isHDR", false);
    }
    PBRShader->setUniform1i("envMap", 1);
    
    // send common matricies
    PBRShader->setUniformMatrix4f("viewTranspose", ofMatrix4x4::getTransposedOf(camera->getModelViewMatrix()));
    PBRShader->setUniformMatrix4f("viewMatrix", ofGetCurrentViewMatrix());
    
    // enable lights
    PBRShader->setUniform1i("numLights", lights.size());
	PBRShader->setUniform1i("numShadowMaps", normalLights.size());
	PBRShader->setUniform1i("numOmniShadowMaps", pointLights.size());

    for (int i = 0; i<normalLights.size(); i++) {
        if (cubeMap != nullptr && normalLights[i]->getLightType() == LightType_Sky && normalLights[i]->getShadowType() != ShadowType_None){
			normalLights[i]->setSkyLightRotation(cubeMap->getRotation());
        }
        shadowMatrix[i] = normalLights[i]->getShadowMatrix(camera->getModelViewMatrix());
		normalLights[i]->beginLighting(PBRShader);
    }

	for (int i = 0; i < pointLights.size(); i++) {
		pointLights[i]->beginLighting(PBRShader);
	}
    
    PBRShader->setUniform1i("shadowMap", 10);
    PBRShader->setUniform1i("omniShadowMap", 11);
    
    // send depth maps
    if (normalLights.size() != 0) {
        shadow.bind(10);
		PBRShader->setUniform2f("depthMapRes", shadow.getDepthMapResolution(), shadow.getDepthMapResolution());
        glUniformMatrix4fv(PBRShader->getUniformLocation("shadowMatrix"), normalLights.size(), false, shadowMatrix[0].getPtr());
    }

	if (pointLights.size() != 0) {
        omniShadow.bind(11);
    }
}

void ofxPBR::endPBR(){
    // end lighting
    for (int i = 0; i<normalLights.size(); i++) {
		normalLights[i]->endLighting(PBRShader);
    }
    
    PBRShader->end();
    
    // unbind cube map
    if (enableCubemap && cubeMap !=nullptr && cubeMap->isAllocated()) {
        cubeMap->unbind();
    }
    
    // unbind depth map
    if (normalLights.size() != 0) {
        shadow.unbind();
    } 

	if (pointLights.size() != 0) {
		omniShadow.unbind();
	}
}

void ofxPBR::beginDepthMap(int index){
    // render depth maps for shadows
    PBRShader->begin();
    PBRShader->setUniform1i("renderDepthMap", true);
	PBRShader->setUniform1i("renderDepthCubeMap", false);
	PBRShader->setUniformMatrix4f("lightsViewProjectionMatrix", normalLights[index]->getViewProjectionMatrix());
}

void ofxPBR::endDepthMap(){
    PBRShader->end();
}

void ofxPBR::beginDepthCubeMap(int index, int face)
{
 	PBRShader->begin();
	PBRShader->setUniform1i("renderDepthMap", true);
	PBRShader->setUniform1i("renderDepthCubeMap", true);
	PBRShader->setUniform3f("lightPos", pointLights[index]->getPosition());
	PBRShader->setUniform1f("farPlane", pointLights[index]->getFarClip());
	PBRShader->setUniformMatrix4f("lightsViewProjectionMatrix", pointLights[index]->getViewProjectionMatrix(face));
}

void ofxPBR::endDepthCubeMap()
{
	PBRShader->end();
}

void ofxPBR::setLights() {
	for (int i = 0; i < lights.size(); i++) {
		lights[i]->setId(i);
	}

	int omniShadowMapCount = 0;
	int shadowMapCount = 0;
	normalLights.clear();
	pointLights.clear();

	for (auto &light : lights) {
		if (light->getLightType() == LightType_Point) {
			light->setOmniShadowIndex(omniShadowMapCount);
			pointLights.push_back(light);
			omniShadowMapCount++;
		}else{
			light->setShadowIndex(shadowMapCount);
			normalLights.push_back(light);
			shadowMapCount++;
		}
	}
	shadow.setNumLights(shadowMapCount);
	shadowMatrix.assign(shadowMapCount, ofMatrix4x4());
	lightViewProjMatrix.assign(shadowMapCount, ofMatrix4x4());

	omniShadow.setNumLights(omniShadowMapCount);
}

ofTexture* ofxPBR::getDepthMap(int index){
    shadow.bind(0);
    depthThumbnailFbo.begin();
    ofClear(0);
    depthThumbnailShader.begin();
    depthThumbnailShader.setUniform1i("rawSampler", 0);
    depthThumbnailShader.setUniform1i("index", index);
    ofDrawPlane(depthThumbnailFbo.getWidth() / 2, depthThumbnailFbo.getHeight() / 2, depthThumbnailFbo.getWidth(), depthThumbnailFbo.getHeight());
    depthThumbnailShader.end();
    depthThumbnailFbo.end();
    shadow.unbind();
    return &depthThumbnailFbo.getTexture();
}