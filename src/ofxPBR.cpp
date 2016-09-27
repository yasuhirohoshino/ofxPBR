#include "ofxPBR.h"

ofxPBR::ofxPBR(){
}

void ofxPBR::setup(int depthMapResolution) {
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
}

void ofxPBR::begin(ofCamera * camera, ofShader * shader){
    if(shader != nullptr){
        PBRShader = shader;
    }else{
        PBRShader = &defaultShader;
    }

	switch (renderMode)
	{
	case Mode_PBR:
		beginPBR(camera);
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

void ofxPBR::makeDepthMap(function<void()> scene)
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

//void ofxPBR::makeDepthMap(function<void()> scene)
//{
//	depthMapMode = true;
//	shadow.beginDepthMap();
//	scene();
//	shadow.endDepthMap();
//	depthMapMode = false;
//}

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

void ofxPBR::beginPBR(ofCamera * camera){
    // pbr
    PBRShader->begin();
    PBRShader->setUniform1i("renderForDepthMap", false);
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

    for (int i = 0; i<normalLights.size(); i++) {
        if (cubeMap != nullptr && normalLights[i]->getLightType() == LightType_Sky && normalLights[i]->getShadowType() != ShadowType_None){
			normalLights[i]->setSkyLightRotation(cubeMap->getRotation());
        }
        shadowMatrix[i] = normalLights[i]->getShadowMatrix(camera->getModelViewMatrix());
		normalLights[i]->beginLighting(PBRShader, i);
    }

	for (int i = 0; i < pointLights.size(); i++) {
		pointLights[i]->beginLighting(PBRShader, i);
	}
    
    // send depth maps
    if (normalLights.size() != 0) {
        shadow.bind(10);
        PBRShader->setUniform1i("shadowMap", 10);
		PBRShader->setUniform2f("depthMapRes", shadow.getDepthMapResolution(), shadow.getDepthMapResolution());
        glUniformMatrix4fv(PBRShader->getUniformLocation("shadowMatrix"), lights.size(), false, shadowMatrix[0].getPtr());
    }

	if (pointLights.size() != 0) {
		omniShadow.bind(0, 11);
		PBRShader->setUniform1i("omniShadowMap", 11);
		//PBRShader->setUniform2f("omniDepthMapRes", omniShadow.getDepthMapResolution(), omniShadow.getDepthMapResolution());
		//glUniformMatrix4fv(PBRShader->getUniformLocation("shadowMatrix"), lights.size(), false, shadowMatrix[0].getPtr());
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
		omniShadow.unbind(11);
	}
}

void ofxPBR::beginDepthMap(int index){
    // render depth maps for shadows
    PBRShader->begin();
    PBRShader->setUniform1i("renderForDepthMap", true);
	PBRShader->setUniformMatrix4f("viewMat", normalLights[index]->getViewProjectionMatrix());
}

//void ofxPBR::beginDepthMap() {
//	// render depth maps for shadows
//	PBRShader->begin();
//	PBRShader->setUniform1i("renderForDepthMap", true);
//	for (int i = 0; i < lights.size(); i++) {
//		lightViewProjMatrix[i] = lights[i]->getViewProjectionMatrix();
//	}
//	PBRShader->setUniform1i("numLights", lights.size());
//	glUniformMatrix4fv(PBRShader->getUniformLocation("viewMat"), lights.size(), false, lightViewProjMatrix[0].getPtr());
//}

void ofxPBR::endDepthMap(){
    PBRShader->end();
}

void ofxPBR::beginDepthCubeMap(int index, int face)
{
 	PBRShader->begin();
	PBRShader->setUniform1i("renderForDepthMap", true);
	PBRShader->setUniform3f("lightPos", pointLights[index]->getPosition());
	PBRShader->setUniform1f("farPlane", pointLights[index]->getFarClip());
	PBRShader->setUniformMatrix4f("viewMat", pointLights[index]->getViewProjectionMatrix(face));
}

void ofxPBR::endDepthCubeMap()
{
	PBRShader->end();
}

void ofxPBR::setNumLights(int numLights) {
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