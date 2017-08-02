#include "ofxPBRCascadeShadow.h"

void ofxPBRCascadeShadow::setup(int maxShadow, vector<float> clipDistances, int resolution)
{
	this->maxShadow = maxShadow;
	this->clipDistances = clipDistances;
	this->numCascade = clipDistances.size() + 1;
	this->depthMapRes = resolution;

	// frustom
	frustomData.resize(numCascade);
	clips.resize(numCascade + 1);

	// shadow matrix
	shadowMatrix.resize(maxShadow * numCascade);

	cascadeShadowData.resize(maxShadow);
	for (auto &c : cascadeShadowData) {
		c.resize(numCascade);
	}

	initFbo();
}

void ofxPBRCascadeShadow::resizeDepthMap(int resolution)
{
	depthMapRes = resolution;
	initFbo();
}

void ofxPBRCascadeShadow::setMaxShadow(int maxShadow)
{
	this->maxShadow = maxShadow;
	initFbo();
}

void ofxPBRCascadeShadow::setNumCascade(int numCascade)
{
	this->numCascade = numCascade;
	initFbo();
}

void ofxPBRCascadeShadow::updateCameraFrustom(ofCamera * cam)
{
	cameraInverseViewMmatrix = cam->getModelViewMatrix().getInverse();
	cameraViewProjectionMatrix = cam->getModelViewProjectionMatrix(ofRectangle(0, 0, depthMapRes, depthMapRes));

	clips[0] = cam->getNearClip();
	for (int i = 1; i < clips.size() - 1; i++) {
		clips[i] = cam->getNearClip() + (cam->getFarClip() - cam->getNearClip()) * clipDistances[i - 1];
	}
	clips.back() = cam->getFarClip();

	for (int i = 0; i < frustomData.size(); i++) {
		frustomData[i].nearClip = clips[i];
		frustomData[i].farClip = clips[i + 1];

		float nearY = clips[i] * tan(ofDegToRad(cam->getFov() / 2));
		float farY = clips[i + 1] * tan(ofDegToRad(cam->getFov() / 2));
		float nearX = nearY * cam->getAspectRatio();
		float farX = farY * cam->getAspectRatio();

		frustomData[i].corners[0] = ofVec3f(-nearX, -nearY, -frustomData[i].nearClip);
		frustomData[i].corners[1] = ofVec3f(nearX, -nearY, -frustomData[i].nearClip);
		frustomData[i].corners[2] = ofVec3f(nearX, nearY, -frustomData[i].nearClip);
		frustomData[i].corners[3] = ofVec3f(-nearX, nearY, -frustomData[i].nearClip);
		frustomData[i].corners[4] = ofVec3f(-farX, -farY, -frustomData[i].farClip);
		frustomData[i].corners[5] = ofVec3f(farX, -farY, -frustomData[i].farClip);
		frustomData[i].corners[6] = ofVec3f(farX, farY, -frustomData[i].farClip);
		frustomData[i].corners[7] = ofVec3f(-farX, farY, -frustomData[i].farClip);

		ofVec3f frustomCenter = ofVec3f(0.0);
		for (int j = 0; j < 8; j++) {
			frustomCenter += frustomData[i].corners[j];
		}
		frustomCenter /= 8;

		for (int j = 0; j < 8; j++) {
			frustomData[i].corners[j] += (frustomData[i].corners[j] - frustomCenter).getNormalized() * 200;
		}
	}
}

void ofxPBRCascadeShadow::updateLightMatrix(int index, ofCamera * light)
{
	ofMatrix4x4 lightMatrix;
	lightMatrix.makeLookAtViewMatrix(ofVec3f(0.0), -light->getLookAtDir(), ofVec3f(0.0, 1.0, 0.0));
	ofMatrix4x4 inverseLightViewMatrix = lightMatrix.getInverse();
	for (int i = 0; i < numCascade; i++) {
		float minX, minY, minZ, maxX, maxY, maxZ;
		minX = FLT_MAX;
		minY = FLT_MAX;
		minZ = FLT_MAX;
		maxX = FLT_MIN;
		maxY = FLT_MIN;
		maxZ = FLT_MIN;

		for (int j = 0; j < 8; j++) {
			ofVec3f AABBCorner = ofVec3f(ofVec4f(frustomData[i].corners[j].x, frustomData[i].corners[j].y, frustomData[i].corners[j].z, 1.0) * cameraInverseViewMmatrix);
			AABBCorner = ofVec3f(ofVec4f(AABBCorner.x, AABBCorner.y, AABBCorner.z, 1.0) * lightMatrix);
			minX = fminf(minX, AABBCorner.x);
			minY = fminf(minY, AABBCorner.y);
			minZ = fminf(minZ, AABBCorner.z);
			maxX = fmaxf(maxX, AABBCorner.x);
			maxY = fmaxf(maxY, AABBCorner.y);
			maxZ = fmaxf(maxZ, AABBCorner.z);
		}

		ofVec3f AABBCorners[8];
		AABBCorners[0] = ofVec3f(ofVec4f(minX, minY, minZ, 1.0) * inverseLightViewMatrix);
		AABBCorners[1] = ofVec3f(ofVec4f(maxX, minY, minZ, 1.0) * inverseLightViewMatrix);
		AABBCorners[2] = ofVec3f(ofVec4f(maxX, maxY, minZ, 1.0) * inverseLightViewMatrix);
		AABBCorners[3] = ofVec3f(ofVec4f(minX, maxY, minZ, 1.0) * inverseLightViewMatrix);
		AABBCorners[4] = ofVec3f(ofVec4f(minX, minY, maxZ, 1.0) * inverseLightViewMatrix);
		AABBCorners[5] = ofVec3f(ofVec4f(maxX, minY, maxZ, 1.0) * inverseLightViewMatrix);
		AABBCorners[6] = ofVec3f(ofVec4f(maxX, maxY, maxZ, 1.0) * inverseLightViewMatrix);
		AABBCorners[7] = ofVec3f(ofVec4f(minX, maxY, maxZ, 1.0) * inverseLightViewMatrix);

		float sx = (ofVec3f(maxX, minY, minZ) - ofVec3f(minX, minY, minZ)).length() / depthMapRes;
		float sy = (ofVec3f(minX, maxY, minZ) - ofVec3f(minX, minY, minZ)).length() / depthMapRes;
		float farClip = (ofVec3f(minX, minY, maxZ) - ofVec3f(minX, minY, minZ)).length();

		ofVec3f position = (AABBCorners[0] + AABBCorners[1] + AABBCorners[2] + AABBCorners[3]) / 4;
		ofVec3f target = (AABBCorners[4] + AABBCorners[5] + AABBCorners[6] + AABBCorners[7]) / 4;
		ofVec3f dir = (target - position).getNormalized();

		cascadeShadowData[index][i].depthCamera.setVFlip(false);
		cascadeShadowData[index][i].depthCamera.enableOrtho();
		cascadeShadowData[index][i].depthCamera.setPosition(position - dir * 100);
		cascadeShadowData[index][i].depthCamera.lookAt(target);
		cascadeShadowData[index][i].depthCamera.setNearClip(-100);
		cascadeShadowData[index][i].depthCamera.setFarClip(farClip + 100);
		cascadeShadowData[index][i].depthCamera.setScale(sx * 1.1, sy * 1.1, 1);

		cascadeShadowData[index][i].shadowMatrix = cascadeShadowData[index][i].depthCamera.getModelViewProjectionMatrix(ofRectangle(0, 0, depthMapRes, depthMapRes)) * biasMatrix;
		shadowMatrix[index * numCascade + i] = cascadeShadowData[index][i].shadowMatrix;
	}
}

void ofxPBRCascadeShadow::beginDepthMap(int index, int cascade)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texIndex, 0, index * numCascade + cascade);
	ofPushView();
	ofViewport(0, 0, depthMapRes, depthMapRes, false);
	ofClear(0);
	glDrawBuffer(GL_NONE);
}

void ofxPBRCascadeShadow::endDepthMap()
{
	ofPopView();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ofxPBRCascadeShadow::bind(GLuint location)
{
	this->location = location;
	glActiveTexture(GL_TEXTURE0 + location);
	glEnable(GL_TEXTURE_2D_ARRAY);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texIndex);
}

void ofxPBRCascadeShadow::unbind()
{
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	glDisable(GL_TEXTURE_2D_ARRAY);
	glActiveTexture(GL_TEXTURE0);
}

void ofxPBRCascadeShadow::initFbo()
{
	glGenTextures(1, &texIndex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texIndex);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, depthMapRes, depthMapRes, maxShadow * numCascade, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texIndex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}