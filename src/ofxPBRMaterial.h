#pragma once
#include "ofMain.h"
#include "ofxPBR.h"
class ofxPBR;
class ofxPBRMaterial{
public:
    bool enableBaseColorMap = true;
    bool enableRoughnessMap = true;
    bool enableMetallicMap = true;
    bool enableNormalMap = true;
    bool enableOcclusionMap = true;
    bool enableEmissionMap = true;
    bool enableDetailBaseColorMap = true;
    bool enableDetailNormalMap = true;
    bool enableGlobalColor = false;
        
    ofTexture* baseColorMap;
    ofTexture* roughnessMap;
    ofTexture* metallicMap;
    ofTexture* normalMap;
    ofTexture* occlusionMap;
    ofTexture* emissionMap;
    ofTexture* detailBaseColorMap;
    ofTexture* detailNormalMap;

    ofFloatColor baseColor = ofFloatColor(1);
    float roughness = 0.0;
    float metallic = 0.0;
    float normalVal = 1.0;
    
	ofVec2f textureRepeat = ofVec2f(1.0, 1.0);
    ofVec2f detailTextureRepeat = ofVec2f(1.0, 1.0);
    
    void begin(ofxPBR * pbr);
    void end();
private:
    ofShader* shader;
	ofxPBR* pbr;
};