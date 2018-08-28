#include "ofxPBRMaterial.h"

void ofxPBRMaterial::begin(ofxPBR * pbr){
	if (pbr->getRenderMode() == Mode_PBR) {
		this->shader = pbr->getShader();
		shader->setUniform2f("textureRepeatTimes", textureRepeat);
		shader->setUniform2f("detailTextureRepeatTimes", detailTextureRepeat);

		// baseColor
		if (enableGlobalColor) {
			shader->setUniform1i("enableGlobalColor", 1);
		}
		else {
			shader->setUniform1i("enableGlobalColor", 0);
			shader->setUniform4f("baseColorUniform", baseColor);
		}

		if (enableBaseColorMap && baseColorMap != nullptr && baseColorMap->isAllocated()) {
			shader->setUniform1i("enableBaseColorMap", 1);
			shader->setUniformTexture("baseColorMap", *baseColorMap, 2);
		}
		else {
			shader->setUniform1i("enableBaseColorMap", 0);
		}

		// roughnessMap
		if (enableRoughnessMap && roughnessMap != nullptr && roughnessMap->isAllocated()) {
			shader->setUniform1i("enableRoughnessMap", 1);
			shader->setUniformTexture("roughnessMap", *roughnessMap, 3);
		}
		else {
			shader->setUniform1i("enableRoughnessMap", 0);
			shader->setUniform1f("roughnessUniform", roughness);
		}

		// metallicMap
		if (enableMetallicMap && metallicMap != nullptr && metallicMap->isAllocated()) {
			shader->setUniform1i("enableMetallicMap", 1);
			shader->setUniformTexture("metallicMap", *metallicMap, 4);
		}
		else {
			shader->setUniform1i("enableMetallicMap", 0);
			shader->setUniform1f("metallicUniform", metallic);
		}

		// normalMap
		if (enableNormalMap && normalMap != nullptr && normalMap->isAllocated()) {
			shader->setUniform1i("enableNormalMap", 1);
			shader->setUniformTexture("normalMap", *normalMap, 5);
			shader->setUniform1f("normalValUniform", normalVal);
		}
		else {
			shader->setUniform1i("enableNormalMap", 0);
		}

		// occlusionMap
		if (enableOcclusionMap && occlusionMap != nullptr && occlusionMap->isAllocated()) {
			shader->setUniform1i("enableOcclusionMap", 1);
			shader->setUniformTexture("occlusionMap", *occlusionMap, 6);
		}
		else {
			shader->setUniform1i("enableOcclusionMap", 0);
		}

		// emissionMap
		if (enableEmissionMap && emissionMap != nullptr && emissionMap->isAllocated()) {
			shader->setUniform1i("enableEmissionMap", 1);
			shader->setUniformTexture("emissionMap", *emissionMap, 7);
		}
		else {
			shader->setUniform1i("enableEmissionMap", 0);
		}

		// detailBaseColor
		if (enableDetailBaseColorMap && detailBaseColorMap != nullptr && detailBaseColorMap->isAllocated()) {
			shader->setUniform1i("enableDetailBaseColorMap", 1);
			shader->setUniformTexture("detailBaseColorMap", *detailBaseColorMap, 8);
		}
		else {
			shader->setUniform1i("enableDetailBaseColorMap", 0);
		}

		// detailNormalMap
		if (enableDetailNormalMap && detailNormalMap != nullptr && detailNormalMap->isAllocated()) {
			shader->setUniform1i("enableDetailNormalMap", 1);
			shader->setUniformTexture("detailNormalMap", *detailNormalMap, 9);
		}
		else {
			shader->setUniform1i("enableDetailNormalMap", 0);
		}
	}
	else {
		this->shader = nullptr;
	}
}

void ofxPBRMaterial::end(){
	if (shader != nullptr) {
		shader->setUniform1i("enableBaseColorMap", 0);
		shader->setUniform1i("enableRoughnessMap", 0);
		shader->setUniform1i("enableMetallicMap", 0);
		shader->setUniform1i("enableNormalMap", 0);
		shader->setUniform1i("enableOcclusionMap", 0);
		shader->setUniform1i("enableEmissionMap", 0);
		shader->setUniform1i("enableDetailBaseColorMap", 0);
		shader->setUniform1i("enableDetailNormalMap", 0);
		shader->setUniform1i("enableGlobalColor", 0);
		shader->setUniform2f("textureRepeatTimes", ofVec2f(1.0, 1.0));
		shader->setUniform2f("detailTextureRepeatTimes", ofVec2f(1.0, 1.0));
	}
}