#pragma once
#include "ofMain.h"
#include "ofxPBR.h"
class ofxPBR;
class ofxPBRMaterial{
public:
    bool enableBaseColorMap = false;
    bool enableRoughnessMap = false;
    bool enableMetallicMap = false;
    bool enableNormalMap = false;
    bool enableOcclusionMap = false;
    bool enableEmissionMap = false;
    bool enableDetailBaseColorMap = false;
    bool enableDetailNormalMap = false;
    bool enableGlobalColor = false;
    
    GLuint baseColorMapIndex;
    GLuint roughnessMapIndex;
    GLuint metallicMapIndex;
    GLuint normalMapIndex;
    GLuint occlusionMapIndex;
    GLuint emissionMapIndex;
    GLuint detailBaseColorMapIndex;
    GLuint DetailNormalMapIndex;
    GLuint GlobalColorIndex;
        
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
    
    void begin(ofxPBR* pbr);
    void begin(ofShader* shader);
    void end();
private:
    ofShader* shader;
};