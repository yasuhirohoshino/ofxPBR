#include "ofxPBR.h"

ofxPBR::ofxPBR(){
}

void ofxPBR::setup(function<void()> scene, ofCamera* camera, int depthMapResolution)
{
	this->scene = scene;
	this->camera = camera;

	sphereMesh = ofSpherePrimitive(1, 100).getMesh();
	for (int i = 0; i<sphereMesh.getNormals().size(); i++) {
		sphereMesh.setNormal(i, ofVec3f(1.0, 1.0, -1.0) * glm::normalize(sphereMesh.getVertex(i)));
	}

	spotShadow.setup(4, depthMapResolution);
	omniShadow.setup(1, depthMapResolution);
	directionalShadow.setup(1, depthMapResolution);
	directionalShadow.setBoundingBox(0, 0, 0, 1000, 1000, 1000);
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
		beginSpotDepthMap();
		break;

	case Mode_OmniShadow:
		beginDepthCubeMap();
		break;

	case Mode_DirectionalShadow:
		beginDirectionalDepthMap();
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
	case Mode_OmniShadow:
	case Mode_DirectionalShadow:
		PBRShader->end();
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
void ofxPBR::setCameraForDirectionalShadowBBox(ofCamera* camera)
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

	glm::mat4 projectionMatrix = ofGetCurrentMatrix(OF_MATRIX_PROJECTION);
	float m22 = projectionMatrix[2][2];
	float m32 = projectionMatrix[3][2];
\
	float nearClip = (2.0f*m32) / (2.0f*m22 - 2.0f);
	float farClip = ((m22 - 1.0f) * nearClip) / (m22 + 1.0);

	if (enableCubemap && cubeMap != nullptr && cubeMap->isAllocated()) {
		float scale = (farClip - nearClip) / 2;
		glm::mat4 invCurrentViewMatrix = glm::inverse(ofGetCurrentViewMatrix());
		glm::vec3 translate = glm::vec3(invCurrentViewMatrix[3][0], invCurrentViewMatrix[3][1], invCurrentViewMatrix[3][2]);

		ofDisableDepthTest();
		ofPushMatrix();
		ofTranslate(translate);
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
	spotShadow.setMaxShadow(maxShadow);
}

void ofxPBR::setMaxOmniShadow(int maxOmniShadow)
{
	omniShadow.setMaxShadow(maxOmniShadow);
}

void ofxPBR::setMaxDirectionalShadow(int maxDirectionalShadow)
{
	directionalShadow.setMaxShadow(maxDirectionalShadow);
}

// depth map
void ofxPBR::resizeDepthMap(int resolution){
    spotShadow.resizeDepthMap(resolution);
}

void ofxPBR::setDirectionalShadowBBox(float x, float y, float z, float width, float height, float depth)
{
	directionalShadow.setBoundingBox(x, y, x, width, height, depth);
}

void ofxPBR::setUsingCameraFrustomForShadow(bool usingCameraFrustom)
{
	directionalShadow.setUsingCameraFrustom(usingCameraFrustom);
}

void ofxPBR::updateDepthMaps()
{
	ofPushStyle();
	ofEnableDepthTest();

	spotShadowIndex = 0;
	omniShadowIndex = 0;
	directionalShadowIndex = 0;

	for (int i = 0; i < lights.size(); i++) {
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

				if (directionalShadowIndex < directionalShadow.getMaxShadow()) {
					if (directionalShadowIndex == 0) {
						directionalShadow.calcCorners(camera);
					}
					renderMode = Mode_DirectionalShadow;
					lights[i]->setDirectionalShadowIndex(directionalShadowIndex);
					directionalShadow.beginDepthMap(directionalShadowIndex, lights[i]);
					scene();
					directionalShadow.endDepthMap();
					directionalShadowIndex++;
				}
			}
				break;

			// spot shadow
			case LightType_Spot:
			{
				if (spotShadowIndex < spotShadow.getMaxShadow()) {
					renderMode = Mode_SpotShadow;
					lights[i]->setSpotShadowIndex(spotShadowIndex);
					spotShadow.beginDepthMap(spotShadowIndex, lights[i]);
					scene();
					spotShadow.endDepthMap();
					spotShadowIndex++;
				}
			}
				break;

			// omni-shadow
			case LightType_Point:
			{
				if (omniShadowIndex < omniShadow.getMaxShadow()) {
					currentLightIndex = i;
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

	glm::mat4 modelViewMatrix = ofGetCurrentMatrix(ofMatrixMode::OF_MATRIX_MODELVIEW);
	
	// pbr
    PBRShader->begin();

	// common uniforms
	PBRShader->setUniform1i("renderMode", renderMode);
	PBRShader->setUniformMatrix4f("viewTranspose", glm::transpose(modelViewMatrix));
	PBRShader->setUniformMatrix4f("viewMatrix", modelViewMatrix);

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
		PBRShader->setUniform3f("lights[" + lightIndex + "].vPosition", lights[i]->getViewSpacePosition(modelViewMatrix));
		PBRShader->setUniform3f("lights[" + lightIndex + "].direction", lights[i]->getViewSpaceDirection(modelViewMatrix));
		// shadow
		PBRShader->setUniform1i("lights[" + lightIndex + "].shadowType", lights[i]->getShadowType());
		PBRShader->setUniform1i("lights[" + lightIndex + "].spotShadowIndex", lights[i]->getSpotShadowIndex());
		PBRShader->setUniform1i("lights[" + lightIndex + "].omniShadowIndex", lights[i]->getOmniShadowIndex());
		PBRShader->setUniform1i("lights[" + lightIndex + "].directionalShadowIndex", lights[i]->getDirectionalShadowIndex());
		PBRShader->setUniform1f("lights[" + lightIndex + "].shadowStrength", lights[i]->getShadowStrength());
		PBRShader->setUniform1f("lights[" + lightIndex + "].bias", lights[i]->getShadowBias());
		PBRShader->setUniform1f("lights[" + lightIndex + "].farClip", lights[i]->getFarClip());
		// point light
		PBRShader->setUniform1f("lights[" + lightIndex + "].pointLightRadius", lights[i]->getPointLightRadius());
		// spot light
		PBRShader->setUniform1f("lights[" + lightIndex + "].spotLightFactor", lights[i]->getSpotLightGradient());
		PBRShader->setUniform1f("lights[" + lightIndex + "].spotLightCutoff", lights[i]->getSpotLightCutoff());
		PBRShader->setUniform1f("lights[" + lightIndex + "].spotLightDistance", lights[i]->getSpotLightDistance());
	}
    
    // depth map uniforms
	directionalShadow.bind(10);
	PBRShader->setUniform1i("directionalShadowMap", 10);
	glUniformMatrix4fv(PBRShader->getUniformLocation("directionalShadowMatrix"), directionalShadow.getShadowMatrix().size(), false, &directionalShadow.getShadowMatrix()[0][0][0]);

	spotShadow.bind(11);
	PBRShader->setUniform1i("spotShadowMap", 11);
	glUniformMatrix4fv(PBRShader->getUniformLocation("spotShadowMatrix"), spotShadow.getShadowMatrix().size(), false, &spotShadow.getShadowMatrix()[0][0][0]);

	omniShadow.bind(12);
	PBRShader->setUniform1i("omniShadowMap", 12);
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
	directionalShadow.unbind();
	
    spotShadow.unbind();

	omniShadow.unbind();
}

// render depth map for shadow
void ofxPBR::beginSpotDepthMap(){
    // render depth maps for shadows
    PBRShader->begin();
	PBRShader->setUniform1i("renderMode", renderMode);
	PBRShader->setUniformMatrix4f("lightsViewProjectionMatrix", spotShadow.getViewProjMatrix(spotShadowIndex));
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

void ofxPBR::beginDirectionalDepthMap()
{
	PBRShader->begin();
	PBRShader->setUniform1i("renderMode", renderMode);
	PBRShader->setUniformMatrix4f("lightsViewProjectionMatrix", directionalShadow.getViewProjMatrix(directionalShadowIndex));
}