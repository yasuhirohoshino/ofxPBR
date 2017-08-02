#include "ofxPBR.h"

ofxPBR::ofxPBR(){
}

void ofxPBR::setup(function<void()> scene, ofCamera* camera, int depthMapResolution)
{
	this->scene = scene;
	this->camera = camera;

	sphereMesh = ofSpherePrimitive(1, 100).getMesh();
	for (int i = 0; i<sphereMesh.getNormals().size(); i++) {
		sphereMesh.setNormal(i, ofVec3f(1.0, 1.0, -1.0) * sphereMesh.getVertex(i).normalize());
	}

	cascadeDistances.resize(3);
	cascadeDistances[0] = 0.1;
	cascadeDistances[1] = 0.3;
	cascadeDistances[2] = 0.5;

	shadow.setup(4, depthMapResolution);
	omniShadow.setup(1, depthMapResolution);
	cascadeShadow.setup(2, cascadeDistances, depthMapResolution);
	renderMode = Mode_PBR;

	// load env shader
	envShader = new ofShader();
	envShader->setupShaderFromSource(GL_VERTEX_SHADER, environmentShaderSource.gl3VertShader);
	envShader->setupShaderFromSource(GL_FRAGMENT_SHADER, environmentShaderSource.gl3FragShader);
	envShader->bindDefaults();
	envShader->linkProgram();

	// load defalut pbr shader
	defaultShader = ofShader();
	defaultShader.setupShaderFromSource(GL_VERTEX_SHADER, pbrShaderSource.gl3VertShader);
	defaultShader.setupShaderFromSource(GL_FRAGMENT_SHADER, pbrShaderSource.gl3FragShader);
	defaultShader.bindDefaults();
	defaultShader.linkProgram();

	depthThumbnailFbo.allocate(depthMapResolution, depthMapResolution, GL_R32F);
	depthThumbnailShader = ofShader();
	depthThumbnailShader.setupShaderFromSource(GL_VERTEX_SHADER, depthThumbnail.gl3VertShader);
	depthThumbnailShader.setupShaderFromSource(GL_FRAGMENT_SHADER, depthThumbnail.gl3FragShader);
	depthThumbnailShader.bindDefaults();
	depthThumbnailShader.linkProgram();
}

// render shader
void ofxPBR::beginDefaultRenderer()
{
	PBRShader = &defaultShader;
	begin();
}

void ofxPBR::endDefaultRenderer()
{
	end();
}

void ofxPBR::beginCustomRenderer(ofShader* shader)
{
	PBRShader = shader;
	begin();
}

void ofxPBR::endCustomRenderer()
{
	PBRShader = &defaultShader;
	end();
}

void ofxPBR::begin(){
	switch (renderMode) {
	case Mode_PBR:
		beginPBR();
		break;

	case Mode_SpotShadow:
		beginDepthMap();
		break;

	case Mode_OmniShadow:
		beginDepthCubeMap();
		break;

	case Mode_CascadeShadow:
		beginCascadeDepthMap();
		break;

	default:
		break;
	}
}

void ofxPBR::end(){
	switch (renderMode) {
	case Mode_PBR:
		endPBR();
		break;

	case Mode_SpotShadow:
		endDepthMap();
		break;

	case Mode_OmniShadow:
		endDepthCubeMap();
		break;

	case Mode_CascadeShadow:
		endCascadeDepthMap();
		break;

	default:
		break;
	}
}

void ofxPBR::renderScene()
{
	if (enableDrawEnvironment) {
		drawEnvironment();
	}
	scene();
}

// camera
void ofxPBR::setCamera(ofCamera* camera)
{
	this->camera = camera;
}

// cubemap
void ofxPBR::setCubeMap(ofxPBRCubeMap* cubeMap)
{
	this->cubeMap = cubeMap;
	enableCubemap = true;
}

void ofxPBR::enableCubeMap(bool enable)
{
	enableCubemap = enable;
}

// environment
void ofxPBR::setDrawEnvironment(bool enable)
{
	enableDrawEnvironment = enable;
}

void ofxPBR::drawEnvironment(){
	if (enableCubemap && cubeMap != nullptr && cubeMap->isAllocated()) {
		float scale = (camera->getFarClip() - camera->getNearClip()) / 2;
		ofDisableDepthTest();
		ofPushMatrix();
		ofTranslate(ofGetCurrentViewMatrix().getInverse().getTranslation());
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

void ofxPBR::setEnvShader(ofShader* shader) {
	envShader = shader;
}

void ofxPBR::setMaxShadow(int maxShadow)
{
	shadow.setMaxShadow(maxShadow);
}

void ofxPBR::setMaxOmniShadow(int maxOmniShadow)
{
	omniShadow.setMaxShadow(maxOmniShadow);
}

void ofxPBR::setMaxCascadeShadow(int maxCascadeShadow)
{
	cascadeShadow.setMaxShadow(maxCascadeShadow);
}

// depth map
void ofxPBR::resizeDepthMap(int resolution){
    shadow.resizeDepthMap(resolution);
}

void ofxPBR::updateDepthMaps()
{
	ofPushStyle();
	ofEnableDepthTest();

	if (cascadeShadow.getMaxShadow() != 0) {
		cascadeShadow.updateCameraFrustom(camera);
	}

	shadowIndex = 0;
	omniShadowIndex = 0;
	cascadeShadowIndex = 0;

	for (int i = 0; i < lights.size(); i++) {
		currentLightIndex = i;
		if (lights[i]->getIsEnabled() && lights[i]->getShadowType() != ShadowType_None) {
			switch (lights[i]->getLightType())
			{

			// cascade shadow
			case LightType_Directional:
			case LightType_Sky:
			{
				if (lights[i]->getLightType() == LightType_Sky && cubeMap != nullptr) {
					lights[i]->setSkyLightRotation(cubeMap->getRotation());
				}
				if (cascadeShadowIndex < cascadeShadow.getMaxShadow()) {
					renderMode = Mode_CascadeShadow;
					lights[i]->setCascadeShadowIndex(cascadeShadowIndex);
					cascadeShadow.updateLightMatrix(cascadeShadowIndex, lights[i]);
					for (int j = 0; j < cascadeShadow.getNumCascade(); j++) {
						currentCascade = j;
						cascadeShadow.beginDepthMap(cascadeShadowIndex, currentCascade);
						scene();
						cascadeShadow.endDepthMap();
					}
					cascadeShadowIndex++;
				}
			}
				break;

			// normal shadow
			case LightType_Spot:
			{
				if (shadowIndex < shadow.getMaxShadow()) {
					renderMode = Mode_SpotShadow;
					lights[i]->setShadowIndex(shadowIndex);
					shadow.beginDepthMap(shadowIndex, camera, lights[i]);
					scene();
					shadow.endDepthMap();
					shadowIndex++;
				}
			}
				break;

			// omni-shadow
			case LightType_Point:
			{
				if (omniShadowIndex < omniShadow.getMaxShadow()) {
					renderMode = Mode_OmniShadow;
					lights[i]->setOmniShadowIndex(omniShadowIndex);
					omniShadow.updateMatrix(omniShadowIndex, lights[i]);
					for (int j = 0; j < 6; j++) {
						omniShadowFace = j;
						omniShadow.beginDepthMap(omniShadowIndex, omniShadowFace);
						scene();
						omniShadow.endDepthMap();
					}
					omniShadowIndex++;
				}
			}
				break;

			default:
				break;
			}
		}
	}
	ofDisableDepthTest();
	ofPopStyle();

	renderMode = Mode_PBR;
}

// lights
void ofxPBR::addLight(ofxPBRLight * light) {
	for (auto l : lights) {
		if (l == light) return;
	}
	lights.push_back(light);
}

void ofxPBR::removeLight(int index) {
	lights.erase(lights.begin() + index);
}

// private

// pbr
void ofxPBR::beginPBR(){
	// pbr
    PBRShader->begin();

	// common uniforms
	PBRShader->setUniform1i("renderMode", renderMode);
    PBRShader->setUniform1f("cameraNear", camera->getNearClip());
    PBRShader->setUniform1f("cameraFar", camera->getFarClip());
	PBRShader->setUniformMatrix4f("viewTranspose", ofMatrix4x4::getTransposedOf(camera->getModelViewMatrix()));
	PBRShader->setUniformMatrix4f("viewMatrix", camera->getModelViewMatrix());

    // cubemap uniforms
    if (enableCubemap && cubeMap != nullptr && cubeMap->isAllocated()) {
        // enable cubemap
        cubeMap->bind(1);
        PBRShader->setUniform1i("enableEnv", true);
        PBRShader->setUniform1i("numMips", cubeMap->getNumMips());
        PBRShader->setUniform1f("cubeMapExposure", cubeMap->getExposure());
        PBRShader->setUniform1f("cubeMapRotation", cubeMap->getRotation());
        PBRShader->setUniform1i("isHDR", cubeMap->isHDR());
		PBRShader->setUniform1i("envMap", 1);
    }else{
        // disable cubemap
        PBRShader->setUniform1i("enableEnv", false);
        PBRShader->setUniform1i("isHDR", false);
    }
    
    // light uniforms
    PBRShader->setUniform1i("numLights", lights.size());
	for (int i = 0; i < lights.size(); i++) {
		string lightIndex = ofToString(i);

		// common
		PBRShader->setUniform1i("lights[" + lightIndex + "].isEnabled", lights[i]->getIsEnabled());
		PBRShader->setUniform1i("lights[" + lightIndex + "].type", lights[i]->getLightType());
		// color
		PBRShader->setUniform4f("lights[" + lightIndex + "].color", lights[i]->getColor());
		PBRShader->setUniform1f("lights[" + lightIndex + "].intensity", lights[i]->getIntensity());
		// transform
		PBRShader->setUniform3f("lights[" + lightIndex + "].position", lights[i]->getGlobalPosition());
		PBRShader->setUniform3f("lights[" + lightIndex + "].vPosition", lights[i]->getViewSpacePosition(camera->getModelViewMatrix()));
		PBRShader->setUniform3f("lights[" + lightIndex + "].direction", lights[i]->getViewSpaceDirection(camera->getModelViewMatrix()));
		// shadow
		PBRShader->setUniform1i("lights[" + lightIndex + "].shadowType", lights[i]->getShadowType());
		PBRShader->setUniform1i("lights[" + lightIndex + "].shadowIndex", lights[i]->getShadowIndex());
		PBRShader->setUniform1i("lights[" + lightIndex + "].omniShadowIndex", lights[i]->getOmniShadowIndex());
		PBRShader->setUniform1i("lights[" + lightIndex + "].cascadeShadowIndex", lights[i]->getCascadeShadowIndex());
		PBRShader->setUniform1f("lights[" + lightIndex + "].shadowStrength", lights[i]->getShadowStrength());
		PBRShader->setUniform1f("lights[" + lightIndex + "].bias", lights[i]->getShadowBias());
		PBRShader->setUniform1f("lights[" + lightIndex + "].farClip", lights[i]->getFarClip());
		// point light
		PBRShader->setUniform1f("lights[" + lightIndex + "].pointLightRadius", lights[i]->getDistance());
		// spot light
		PBRShader->setUniform1f("lights[" + lightIndex + "].spotLightFactor", lights[i]->getSpotLightGradient());
		PBRShader->setUniform1f("lights[" + lightIndex + "].spotLightCutoff", lights[i]->getSpotLightCutoff());
		PBRShader->setUniform1f("lights[" + lightIndex + "].spotLightDistance", lights[i]->getDistance());
	}
    
    // depth map uniforms
    if (shadow.getMaxShadow() != 0) {
        shadow.bind(10);
		PBRShader->setUniform2f("depthMapRes", ofVec2f(shadow.getDepthMapResolution()));
        glUniformMatrix4fv(PBRShader->getUniformLocation("shadowMatrix"), shadow.getShadowMatrix().size(), false, shadow.getShadowMatrix()[0].getPtr());
		PBRShader->setUniform1i("shadowMap", 10);
	}

	if (omniShadow.getMaxShadow() != 0) {
        omniShadow.bind(11);
		PBRShader->setUniform1i("omniShadowMap", 11);
    }

	if (cascadeShadow.getMaxShadow() != 0) {
		cascadeShadow.bind(12);
		PBRShader->setUniform1i("cascadeShadowMap", 12);
		PBRShader->setUniform1i("numCascade", cascadeShadow.getNumCascade());
		PBRShader->setUniform1fv("cascadeClips", &cascadeShadow.getClips()[0], cascadeShadow.getClips().size());
		glUniformMatrix4fv(PBRShader->getUniformLocation("cascadeShadowMatrix"), cascadeShadow.getShadowMatrix().size(), false, cascadeShadow.getShadowMatrix()[0].getPtr());
	}
}

void ofxPBR::endPBR(){
    // end lighting
    for (int i = 0; i<lights.size(); i++) {
		PBRShader->setUniform1i("lights[" + ofToString(i) + "].isEnabled", 0);
    }

    PBRShader->end();
    
    // unbind cube map
    if (enableCubemap && cubeMap !=nullptr && cubeMap->isAllocated()) {
        cubeMap->unbind();
    }
    
    // unbind depth map
    if (shadow.getMaxShadow() != 0) {
        shadow.unbind();
    }
	
	if (omniShadow.getMaxShadow() != 0) {
		omniShadow.unbind();
	}

	if (cascadeShadow.getMaxShadow() != 0) {
		cascadeShadow.unbind();
	}
}

// render depth map for shadow
void ofxPBR::beginDepthMap(){
    // render depth maps for shadows
    PBRShader->begin();
	PBRShader->setUniform1i("renderMode", renderMode);
	PBRShader->setUniformMatrix4f("lightsViewProjectionMatrix", shadow.getViewProjMatrix(shadowIndex));
}

void ofxPBR::endDepthMap(){
    PBRShader->end();
}

// render depth cubemap for omni-shadow
void ofxPBR::beginDepthCubeMap()
{
 	PBRShader->begin();
	PBRShader->setUniform1i("renderMode", renderMode);
	PBRShader->setUniform3f("lightPos", lights[currentLightIndex]->getPosition());
	PBRShader->setUniform1f("farPlane", lights[currentLightIndex]->getFarClip());
	PBRShader->setUniformMatrix4f("lightsViewProjectionMatrix", omniShadow.getViewProjMatrix(omniShadowIndex, omniShadowFace));
}

void ofxPBR::endDepthCubeMap()
{
	PBRShader->end();
}

void ofxPBR::beginCascadeDepthMap()
{
	PBRShader->begin();
	PBRShader->setUniform1i("renderMode", renderMode);
	PBRShader->setUniformMatrix4f("lightsViewProjectionMatrix", cascadeShadow.getViewProjMatrix(cascadeShadowIndex, currentCascade));
}

void ofxPBR::endCascadeDepthMap()
{
	PBRShader->end();
}

// get depth map for thumbnail
ofTexture* ofxPBR::getDepthMap(int index){
    //shadow.bind(0);
	//cascadeShadow.bind(0);
 //   depthThumbnailFbo.begin();
 //   ofClear(0);
 //   depthThumbnailShader.begin();
 //   depthThumbnailShader.setUniform1i("rawSampler", 0);
 //   depthThumbnailShader.setUniform1i("index", index);
 //   ofDrawPlane(depthThumbnailFbo.getWidth() / 2, depthThumbnailFbo.getHeight() / 2, depthThumbnailFbo.getWidth(), depthThumbnailFbo.getHeight());
 //   depthThumbnailShader.end();
 //   depthThumbnailFbo.end();
 //   //shadow.unbind();
	//cascadeShadow.unbind();

	shadow.bind(0);
	depthThumbnailFbo.begin();
	ofClear(0);
	depthThumbnailShader.begin();
	depthThumbnailShader.setUniform1i("rawSampler", 0);
	depthThumbnailShader.setUniform1i("index", 0);
	ofDrawPlane(depthThumbnailFbo.getWidth() / 2, depthThumbnailFbo.getHeight() / 2, depthThumbnailFbo.getWidth(), depthThumbnailFbo.getHeight());
	depthThumbnailShader.end();
	depthThumbnailFbo.end();
	shadow.unbind();

    return &depthThumbnailFbo.getTexture();
}