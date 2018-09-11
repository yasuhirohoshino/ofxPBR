#include "ofxPBRDirectionalShadow.h"

void ofxPBRDirectionalShadow::setup(int maxShadow, int resolution)
{
	ofxPBRShadow::setup(maxShadow, resolution);
}

void ofxPBRDirectionalShadow::calcCorners(ofCamera * cam)
{
	if (usingCameraFrustom) {
		cameraInverseViewMmatrix = glm::inverse(cam->getModelViewMatrix());
		float nearClip = cam->getNearClip();
		float farClip = cam->getFarClip();

		float nearY = nearClip * tan(ofDegToRad(cam->getFov() / 2));
		float farY = farClip * tan(ofDegToRad(cam->getFov() / 2));
		float nearX = nearY * cam->getAspectRatio();
		float farX = farY * cam->getAspectRatio();

		corners[0] = ofVec3f(-nearX, -nearY, -nearClip);
		corners[1] = ofVec3f(nearX, -nearY, -nearClip);
		corners[2] = ofVec3f(nearX, nearY, -nearClip);
		corners[3] = ofVec3f(-nearX, nearY, -nearClip);
		corners[4] = ofVec3f(-farX, -farY, -farClip);
		corners[5] = ofVec3f(farX, -farY, -farClip);
		corners[6] = ofVec3f(farX, farY, -farClip);
		corners[7] = ofVec3f(-farX, farY, -farClip);

		ofVec3f frustomCenter = ofVec3f(0.0);
		for (int j = 0; j < 8; j++) {
			frustomCenter += corners[j];
		}
		frustomCenter /= 8;

		for (int j = 0; j < 8; j++) {
			corners[j] += (corners[j] - frustomCenter).getNormalized() * 200;
		}
	}
	else {
		corners[0] = ofVec3f(negX, negY, negZ);
		corners[1] = ofVec3f(posX, negY, negZ);
		corners[2] = ofVec3f(posX, posY, negZ);
		corners[3] = ofVec3f(negX, posY, negZ);
		corners[4] = ofVec3f(negX, negY, posZ);
		corners[5] = ofVec3f(posX, negY, posZ);
		corners[6] = ofVec3f(posX, posY, posZ);
		corners[7] = ofVec3f(negX, posY, posZ);
	}
}

void ofxPBRDirectionalShadow::setBoundingBox(float x, float y, float z, float width, float height, float depth)
{
	BBCenter = ofVec3f(x, y, z);
	BBWidth = width;
	BBHeight = height;
	BBDepth = depth;

	negX = BBCenter.x - BBWidth / 2;
	posX = BBCenter.x + BBWidth / 2;
	negY = BBCenter.y - BBHeight / 2;
	posY = BBCenter.y + BBHeight / 2;
	negZ = BBCenter.z - BBDepth / 2;
	posZ = BBCenter.z + BBDepth / 2;
}

void ofxPBRDirectionalShadow::setUsingCameraFrustom(bool usingCameraFrustom)
{
	this->usingCameraFrustom = usingCameraFrustom;
}

void ofxPBRDirectionalShadow::beginDepthMap(int index, ofCamera * light)
{
	glm::mat4 lightMatrix;
	lightMatrix = glm::lookAt(glm::vec3(0.0), -light->getLookAtDir(), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 inverseLightViewMatrix = glm::inverse(lightMatrix);

	float minX, minY, minZ, maxX, maxY, maxZ;
	minX = FLT_MAX;
	minY = FLT_MAX;
	minZ = FLT_MAX;
	maxX = FLT_MIN;
	maxY = FLT_MIN;
	maxZ = FLT_MIN;

	for (int j = 0; j < 8; j++) {
		glm::vec3 AABBCorner = glm::vec3(cameraInverseViewMmatrix * glm::vec4(corners[j].x, corners[j].y, corners[j].z, 1.0));
		AABBCorner = glm::vec3(lightMatrix * glm::vec4(AABBCorner.x, AABBCorner.y, AABBCorner.z, 1.0));
		minX = fminf(minX, AABBCorner.x);
		minY = fminf(minY, AABBCorner.y);
		minZ = fminf(minZ, AABBCorner.z);
		maxX = fmaxf(maxX, AABBCorner.x);
		maxY = fmaxf(maxY, AABBCorner.y);
		maxZ = fmaxf(maxZ, AABBCorner.z);
	}

	ofVec3f AABBCorners[8];
	AABBCorners[0] = glm::vec3(inverseLightViewMatrix * glm::vec4(minX, minY, minZ, 1.0));
	AABBCorners[1] = glm::vec3(inverseLightViewMatrix * glm::vec4(maxX, minY, minZ, 1.0));
	AABBCorners[2] = glm::vec3(inverseLightViewMatrix * glm::vec4(maxX, maxY, minZ, 1.0));
	AABBCorners[3] = glm::vec3(inverseLightViewMatrix * glm::vec4(minX, maxY, minZ, 1.0));
	AABBCorners[4] = glm::vec3(inverseLightViewMatrix * glm::vec4(minX, minY, maxZ, 1.0));
	AABBCorners[5] = glm::vec3(inverseLightViewMatrix * glm::vec4(maxX, minY, maxZ, 1.0));
	AABBCorners[6] = glm::vec3(inverseLightViewMatrix * glm::vec4(maxX, maxY, maxZ, 1.0));
	AABBCorners[7] = glm::vec3(inverseLightViewMatrix * glm::vec4(minX, maxY, maxZ, 1.0));

	float sx = (ofVec3f(maxX, minY, minZ) - ofVec3f(minX, minY, minZ)).length() / depthMapRes;
	float sy = (ofVec3f(minX, maxY, minZ) - ofVec3f(minX, minY, minZ)).length() / depthMapRes;
	float depthCamFarClip = (ofVec3f(minX, minY, maxZ) - ofVec3f(minX, minY, minZ)).length();

	ofVec3f position = (AABBCorners[0] + AABBCorners[1] + AABBCorners[2] + AABBCorners[3]) / 4;
	ofVec3f target = (AABBCorners[4] + AABBCorners[5] + AABBCorners[6] + AABBCorners[7]) / 4;
	ofVec3f dir = (target - position).getNormalized();

	depthCam.setVFlip(false);
	depthCam.enableOrtho();
	depthCam.setPosition(position);
	depthCam.lookAt(target, glm::vec3(0, 1, 0));
	depthCam.setNearClip(0.01);
	depthCam.setFarClip(depthCamFarClip);
	depthCam.setScale(sx, sy, 1);

	ofxPBRShadow::beginDepthMap(index, &depthCam);
}